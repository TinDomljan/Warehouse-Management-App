#include "cryptomanager.h"

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "databasemanager.h"


QString CryptoManager::hashPlain(const QString& password) {
    return QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString CryptoManager::hashWithSalt(const QString& password, const QString& salt) {
    QByteArray data = password.toUtf8() + salt.toUtf8();
    return QString::fromLatin1(
        QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

// sol
QString CryptoManager::deriveSalt(const QString& username) {
    static const QByteArray SALT_RULE_CONSTANT = "WHSE_SALT_RULE_v1";
    const QByteArray data = username.toLower().toUtf8() + SALT_RULE_CONSTANT;
    const QByteArray hex  =
        QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    return QString::fromLatin1(hex.left(32));  // 32 hex chars za 16 bajtova
}

const QStringList& CryptoManager::candidatePeppers() {

    static const QStringList peppers = {
        "WHSE_PEPPER_DECOY_ALPHA",
        "WAREHOUSE_PEPPER_2024",
        "SECRET_SPICE_NTP",
        "WAREHOUSE_PEPPER_2025",
        PEPPER,                          // ← pravi
        "WAREHOUSE_PEPPER_2027",
        "ZAGREB_TVZ_PEPPER",
        "WHSE_PEPPER_DECOY_OMEGA",
        "FALLBACK_PEPPER_99",
        "LAST_RESORT_PEPPER"
    };
    return peppers;
}

QString CryptoManager::hashWithSaltAndSpecificPepper(const QString& password,
                                                     const QString& salt,
                                                     const QString& pepper) {
    QByteArray data = pepper.toUtf8() + password.toUtf8() + salt.toUtf8();
    return QString::fromLatin1(
        QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

bool CryptoManager::verifyPasswordBruteForcePepper(const QString& password,
                                                   const QString& salt,
                                                   const QString& storedHash) {

    const QStringList& peppers = candidatePeppers();
    for (const QString& pepper : peppers) {
        const QString candidate = hashWithSaltAndSpecificPepper(password, salt, pepper);
        if (candidate == storedHash)
            return true;
    }
    return false;
}



static QString opensslError() {
    unsigned long e = ERR_get_error();
    if (e == 0) return "unknown OpenSSL error";
    char buf[256];
    ERR_error_string_n(e, buf, sizeof(buf));
    return QString::fromLatin1(buf);
}




void CryptoManager::deriveKey(const QString& password, unsigned char* key32) {
    const QByteArray pw = password.toUtf8();
    SHA256(reinterpret_cast<const unsigned char*>(pw.constData()),
           static_cast<size_t>(pw.size()),
           key32);
}


QByteArray CryptoManager::encryptAES(const QByteArray& data,
                                     const QString&    password) {
    unsigned char key[KEY_SIZE];
    deriveKey(password, key);


    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        qWarning() << "[Crypto] RAND_bytes failed:" << opensslError();
        return {};
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "[Crypto] EVP_CIPHER_CTX_new failed";
        return {};
    }


    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
        qWarning() << "[Crypto] EncryptInit failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }


    QByteArray ciphertext(data.size() + IV_SIZE, '\0');
    int outLen = 0;

    if (EVP_EncryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(ciphertext.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char*>(data.constData()),
                          static_cast<int>(data.size())) != 1) {
        qWarning() << "[Crypto] EncryptUpdate failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int finalLen = 0;
    if (EVP_EncryptFinal_ex(ctx,
                             reinterpret_cast<unsigned char*>(ciphertext.data()) + outLen,
                             &finalLen) != 1) {
        qWarning() << "[Crypto] EncryptFinal failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(outLen + finalLen);


    QByteArray result;
    result.reserve(IV_SIZE + ciphertext.size());
    result.append(reinterpret_cast<const char*>(iv), IV_SIZE);
    result.append(ciphertext);
    return result;
}



QByteArray CryptoManager::decryptAES(const QByteArray& encrypted,
                                     const QString&    password) {
    if (encrypted.size() <= IV_SIZE) {
        qWarning() << "[Crypto] Input too short — missing IV or ciphertext";
        return {};
    }

    unsigned char key[KEY_SIZE];
    deriveKey(password, key);


    const auto* iv         = reinterpret_cast<const unsigned char*>(encrypted.constData());
    const auto* ciphertext = iv + IV_SIZE;
    const int   cipherLen  = static_cast<int>(encrypted.size()) - IV_SIZE;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "[Crypto] EVP_CIPHER_CTX_new failed";
        return {};
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
        qWarning() << "[Crypto] DecryptInit failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    QByteArray plaintext(cipherLen, '\0');
    int outLen = 0;

    if (EVP_DecryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(plaintext.data()),
                          &outLen,
                          ciphertext,
                          cipherLen) != 1) {
        qWarning() << "[Crypto] DecryptUpdate failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int finalLen = 0;
    if (EVP_DecryptFinal_ex(ctx,
                             reinterpret_cast<unsigned char*>(plaintext.data()) + outLen,
                             &finalLen) != 1) {

        qWarning() << "[Crypto] DecryptFinal failed (wrong password or corrupted data):"
                   << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(outLen + finalLen);
    return plaintext;
}



static EVP_PKEY* rsaLoadPublicKey(const QString& path) {
    BIO* bio = BIO_new_file(path.toLocal8Bit().constData(), "rb");
    if (!bio) return nullptr;
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return pkey;
}

static EVP_PKEY* rsaLoadPrivateKey(const QString& path) {
    BIO* bio = BIO_new_file(path.toLocal8Bit().constData(), "rb");
    if (!bio) return nullptr;
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return pkey;
}

// generiramo RSA ključ

bool CryptoManager::generateRSAKeyPair(const QString& privateKeyPath,
                                        const QString& publicKeyPath,
                                        int            bits) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        qWarning() << "[Crypto] EVP_PKEY_CTX_new_id failed";
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        qWarning() << "[Crypto] RSA keygen init failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        qWarning() << "[Crypto] RSA keygen failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    EVP_PKEY_CTX_free(ctx);


    BIO* priv = BIO_new_file(privateKeyPath.toLocal8Bit().constData(), "wb");
    if (!priv) {
        EVP_PKEY_free(pkey);
        qWarning() << "[Crypto] Cannot open" << privateKeyPath << "for writing";
        return false;
    }
    const bool privOk =
        PEM_write_bio_PrivateKey(priv, pkey, nullptr, nullptr, 0, nullptr, nullptr) == 1;
    BIO_free(priv);


    BIO* pub = BIO_new_file(publicKeyPath.toLocal8Bit().constData(), "wb");
    if (!pub) {
        EVP_PKEY_free(pkey);
        qWarning() << "[Crypto] Cannot open" << publicKeyPath << "for writing";
        return false;
    }
    const bool pubOk = PEM_write_bio_PUBKEY(pub, pkey) == 1;
    BIO_free(pub);

    EVP_PKEY_free(pkey);

    if (!privOk || !pubOk) {
        qWarning() << "[Crypto] PEM write failed:" << opensslError();
        return false;
    }

    qInfo() << "[Crypto] RSA" << bits << "key pair written to"
            << privateKeyPath << "/" << publicKeyPath;
    return true;
}


QByteArray CryptoManager::encryptRSA(const QString& plaintext,
                                      const QString& publicKeyPath) {

    if (!QFile::exists(publicKeyPath)) {
        qInfo() << "[Crypto] Public key not found — generating key pair";
        if (!generateRSAKeyPair("private.pem", publicKeyPath)) {
            qWarning() << "[Crypto] Auto key generation failed";
            return {};
        }
    }

    EVP_PKEY* pkey = rsaLoadPublicKey(publicKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] Failed to load public key:" << opensslError();
        return {};
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    EVP_PKEY_free(pkey);
    if (!ctx) return {};

    if (EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        qWarning() << "[Crypto] RSA encrypt init failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    const QByteArray input = plaintext.toUtf8();
    const auto*      src   = reinterpret_cast<const unsigned char*>(input.constData());
    const size_t     srcLen = static_cast<size_t>(input.size());


    size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, src, srcLen) <= 0) {
        qWarning() << "[Crypto] RSA encrypt (size query) failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    QByteArray ciphertext(static_cast<int>(outLen), '\0');


    if (EVP_PKEY_encrypt(ctx,
                         reinterpret_cast<unsigned char*>(ciphertext.data()),
                         &outLen,
                         src, srcLen) <= 0) {
        qWarning() << "[Crypto] RSA encrypt failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);
    ciphertext.resize(static_cast<int>(outLen));
    return ciphertext;
}



//RSA-SHA256 potpis

QByteArray CryptoManager::signData(const QByteArray& data,
                                    const QString&    privateKeyPath) {
    EVP_PKEY* pkey = rsaLoadPrivateKey(privateKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] signData: failed to load private key:" << opensslError();
        return {};
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return {};
    }

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        qWarning() << "[Crypto] DigestSignInit failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    if (EVP_DigestSignUpdate(ctx,
                             reinterpret_cast<const unsigned char*>(data.constData()),
                             static_cast<size_t>(data.size())) <= 0) {
        qWarning() << "[Crypto] DigestSignUpdate failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }


    size_t sigLen = 0;
    if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) <= 0) {
        qWarning() << "[Crypto] DigestSignFinal (size) failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    QByteArray sig(static_cast<int>(sigLen), '\0');
    if (EVP_DigestSignFinal(ctx,
                            reinterpret_cast<unsigned char*>(sig.data()),
                            &sigLen) <= 0) {
        qWarning() << "[Crypto] DigestSignFinal failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    sig.resize(static_cast<int>(sigLen));
    return sig;
}



bool CryptoManager::verifySignature(const QByteArray& data,
                                     const QByteArray& signature,
                                     const QString&    publicKeyPath) {
    EVP_PKEY* pkey = rsaLoadPublicKey(publicKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] verifySignature: failed to load public key:" << opensslError();
        return false;
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return false;
    }

    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        qWarning() << "[Crypto] DigestVerifyInit failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    if (EVP_DigestVerifyUpdate(ctx,
                               reinterpret_cast<const unsigned char*>(data.constData()),
                               static_cast<size_t>(data.size())) <= 0) {
        qWarning() << "[Crypto] DigestVerifyUpdate failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    const int rc = EVP_DigestVerifyFinal(
        ctx,
        reinterpret_cast<const unsigned char*>(signature.constData()),
        static_cast<size_t>(signature.size()));

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    if (rc == 1)  return true;   // verifikacija je uspijela
    if (rc == 0)  return false;  // ne
    qWarning() << "[Crypto] DigestVerifyFinal error:" << opensslError();
    return false;
}

// vadimo usere iz database i enkriptamo

QByteArray CryptoManager::encryptUsersJson(const QString& publicKeyPath) {

    const std::vector<User> users = DatabaseManager::instance().getAllUsers();

    //serijaliziramo
    QJsonArray arr;
    for (const auto& u : users) {
        QJsonObject obj;
        obj["username"] = QString::fromStdString(u.getUsername());
        obj["role"]     = QString::fromStdString(u.getRoleAsString());
        arr.append(obj);
    }
    const QByteArray json = QJsonDocument(arr).toJson(QJsonDocument::Compact);


    if (json.size() > 245) {
        qWarning() << "[Crypto] encryptUsersJson: JSON" << json.size()
                   << "B exceeds RSA-PKCS1 limit of 245 B — reduce user count";
        return {};
    }

    EVP_PKEY* pkey = rsaLoadPublicKey(publicKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] encryptUsersJson: cannot load public key:" << opensslError();
        return {};
    }
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    EVP_PKEY_free(pkey);
    if (!ctx) return {};

    if (EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    const auto*  src    = reinterpret_cast<const unsigned char*>(json.constData());
    const size_t srcLen = static_cast<size_t>(json.size());



    size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, src, srcLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    QByteArray ciphertext(static_cast<int>(outLen), '\0');
    if (EVP_PKEY_encrypt(ctx,
                         reinterpret_cast<unsigned char*>(ciphertext.data()),
                         &outLen, src, srcLen) <= 0) {
        qWarning() << "[Crypto] encryptUsersJson: RSA encrypt failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }
    EVP_PKEY_CTX_free(ctx);
    ciphertext.resize(static_cast<int>(outLen));

    qInfo() << "[Crypto] encryptUsersJson: encrypted" << users.size()
            << "users," << json.size() << "B JSON →" << ciphertext.size() << "B ciphertext";
    return ciphertext;
}

QString CryptoManager::decryptUsersJson(const QByteArray& encrypted,
                                         const QString&    privateKeyPath) {
    EVP_PKEY* pkey = rsaLoadPrivateKey(privateKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] decryptUsersJson: cannot load private key:" << opensslError();
        return {};
    }
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    EVP_PKEY_free(pkey);
    if (!ctx) return {};

    if (EVP_PKEY_decrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    const auto*  src    = reinterpret_cast<const unsigned char*>(encrypted.constData());
    const size_t srcLen = static_cast<size_t>(encrypted.size());

    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, src, srcLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    QByteArray plaintext(static_cast<int>(outLen), '\0');
    if (EVP_PKEY_decrypt(ctx,
                         reinterpret_cast<unsigned char*>(plaintext.data()),
                         &outLen, src, srcLen) <= 0) {
        qWarning() << "[Crypto] decryptUsersJson: RSA decrypt failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }
    EVP_PKEY_CTX_free(ctx);
    plaintext.resize(static_cast<int>(outLen));
    return QString::fromUtf8(plaintext);
}



QString CryptoManager::decryptRSA(const QByteArray& ciphertext,
                                   const QString&    privateKeyPath) {
    EVP_PKEY* pkey = rsaLoadPrivateKey(privateKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] Failed to load private key:" << opensslError();
        return {};
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    EVP_PKEY_free(pkey);
    if (!ctx) return {};

    if (EVP_PKEY_decrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        qWarning() << "[Crypto] RSA decrypt init failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    const auto*  src    = reinterpret_cast<const unsigned char*>(ciphertext.constData());
    const size_t srcLen = static_cast<size_t>(ciphertext.size());


    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, src, srcLen) <= 0) {
        qWarning() << "[Crypto] RSA decrypt (size query) failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    QByteArray plaintext(static_cast<int>(outLen), '\0');


    if (EVP_PKEY_decrypt(ctx,
                         reinterpret_cast<unsigned char*>(plaintext.data()),
                         &outLen,
                         src, srcLen) <= 0) {
        qWarning() << "[Crypto] RSA decrypt failed (wrong key or corrupted data):"
                   << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);
    plaintext.resize(static_cast<int>(outLen));
    return QString::fromUtf8(plaintext);
}
