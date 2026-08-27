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
    unsigned char key[KEY_SIZE]; //32 bajta na stogu
    deriveKey(password, key); //pozivamo OpenSSL-ov SHA256() i upise rezultat direktno u kjuc


    unsigned char iv[IV_SIZE]; //novi iv pri svakom pozivu, random
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        qWarning() << "[Crypto] RAND_bytes failed:" << opensslError();
        return {};
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new(); //ctx je kontekst, drzi stanje sifriranja izmedu poziva
    if (!ctx) {
        qWarning() << "[Crypto] EVP_CIPHER_CTX_new failed";
        return {};
    }


    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) { //koji algoritam, koji kljuc, koji IV
        qWarning() << "[Crypto] EncryptInit failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }


    //buffer za izlaz, ulaz + 16 bajtova
    QByteArray ciphertext(data.size() + IV_SIZE, '\0');
    int outLen = 0;

    //Šifrira cijele blokove. outLen je izlazni parametar, OpenSSL kroz njega javi koliko je bajtova stvarno zapisao.
    if (EVP_EncryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(ciphertext.data()), //kamo pisat
                          &outLen, //koliko je zapisano
                          reinterpret_cast<const unsigned char*>(data.constData()), //odakle citat
                          static_cast<int>(data.size())) != 1) { //koliko citat
        qWarning() << "[Crypto] EncryptUpdate failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    //uzmemo 4 zadnja bajta filea i nadopunimo ih  do 16
    //ovdje radimo padding, update je ostavio "rep" koji nije pun blok. Final ga nadopuni
    //po PKCS#7 i zašifrira zadnji blok. Zapisuje se iza onoga što je Update već upisao — otud + outLen.
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
    result.reserve(IV_SIZE + ciphertext.size()); //iv ide na pocetak, [IV][ciphertext]
    result.append(reinterpret_cast<const char*>(iv), IV_SIZE);
    result.append(ciphertext);
    return result;
}


//zrcalna slika ko encryptAES
//provjerava se samo "rep" datoteke
//povjerljivost vs intergirtet
QByteArray CryptoManager::decryptAES(const QByteArray& encrypted,
                                     const QString&    password) {
    //mora imati IV i jos nesto
    if (encrypted.size() <= IV_SIZE) {
        qWarning() << "[Crypto] Input too short — missing IV or ciphertext";
        return {};
    }

    unsigned char key[KEY_SIZE];
    deriveKey(password, key);

    //rascjepavamo
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

    //kad je lozinka kriva
    //vraca besmislene podatke ak je kljuc kriv
    if (EVP_DecryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(plaintext.data()),
                          &outLen,
                          ciphertext,
                          cipherLen) != 1) {
        qWarning() << "[Crypto] DecryptUpdate failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }


    //koliko bajtova je padding trebao bit?
    //je li 167(primjer, samo izmedu 1 i 16) uopce moguc broj
    //jesu li zadnji brojevi svi jednaki
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
    //obrnuto od kako smo zapisivali
    //datoteka → PEM_read → pkey
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
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr); //kontekst za rsa
    if (!ctx) {
        qWarning() << "[Crypto] EVP_PKEY_CTX_new_id failed";
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) { //2048
        qWarning() << "[Crypto] RSA keygen init failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) { //izgenerira par kljuceva odjednom
        qWarning() << "[Crypto] RSA keygen failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    EVP_PKEY_CTX_free(ctx); //nakon sto je ctx izvrsio svoje bacimo ga

    //BIO je basic input output
    //zapisujemo kljuceve u datoteku
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

    EVP_PKEY_free(pkey);//posao  skljucem u memoriji je gotov

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

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) { // 1. postavi SHA-256 + ključ
        qWarning() << "[Crypto] DigestSignInit failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    if (EVP_DigestSignUpdate(ctx,
                             reinterpret_cast<const unsigned char*>(data.constData()), // 2. ubaci podatke
                             static_cast<size_t>(data.size())) <= 0) {
        qWarning() << "[Crypto] DigestSignUpdate failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    //dva final poziva, prvi vrati potrebnu duljinu, drugi potpisuje u alocirani buffer
    size_t sigLen = 0;
    if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) <= 0) { // 3a. koliko treba bajtova?
        qWarning() << "[Crypto] DigestSignFinal (size) failed:" << opensslError();
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    QByteArray sig(static_cast<int>(sigLen), '\0');
    if (EVP_DigestSignFinal(ctx, // 3b. daj potpis
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
    return sig; // 256 bajtova (2048-bit ključ)
}



bool CryptoManager::verifySignature(const QByteArray& data,
                                     const QByteArray& signature,
                                     const QString&    publicKeyPath) {
    EVP_PKEY* pkey = rsaLoadPublicKey(publicKeyPath); //public ovdje ne private
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

    //provjeravamo tocnu velicinu
    if (json.size() > 245) {
        qWarning() << "[Crypto] encryptUsersJson: JSON" << json.size()
                   << "B exceeds RSA-PKCS1 limit of 245 B — reduce user count";
        return {};
    }

    //ucitavamo javni kljuc
    EVP_PKEY* pkey = rsaLoadPublicKey(publicKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] encryptUsersJson: cannot load public key:" << opensslError();
        return {};
    }
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    EVP_PKEY_free(pkey);
    if (!ctx) return {};

    if (EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) { //postavljamo padding za sifriranje
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    const auto*  src    = reinterpret_cast<const unsigned char*>(json.constData()); //json.constData vraca adresu prvog bajta bytearraya
    const size_t srcLen = static_cast<size_t>(json.size()); //duljina



    //pogledamo koliko je kljuc velik
    size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, src, srcLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }
    //napravi QByteArray od 256 bajtova i sve postavi na nulu
    QByteArray ciphertext(static_cast<int>(outLen), '\0');
    //stvarno šifriranje
    if (EVP_PKEY_encrypt(ctx,
                         reinterpret_cast<unsigned char*>(ciphertext.data()),
                         &outLen, src, srcLen) <= 0) {
        qWarning() << "[Crypto] encryptUsersJson: RSA encrypt failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    //ciscenje i resizeanje
    EVP_PKEY_CTX_free(ctx);
    ciphertext.resize(static_cast<int>(outLen));

    qInfo() << "[Crypto] encryptUsersJson: encrypted" << users.size()
            << "users," << json.size() << "B JSON →" << ciphertext.size() << "B ciphertext";
    return ciphertext;
}

QString CryptoManager::decryptUsersJson(const QByteArray& encrypted,
                                         const QString&    privateKeyPath) {

    //loadamo privatni kljuc sada a ne javni
    EVP_PKEY* pkey = rsaLoadPrivateKey(privateKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] decryptUsersJson: cannot load private key:" << opensslError();
        return {};
    }

    //kontekst od postojeceg kljuca
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    EVP_PKEY_free(pkey);
    if (!ctx) return {};


    if (EVP_PKEY_decrypt_init(ctx) <= 0 || //ovaj kontekst ce desifrirat
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {  //padding mora biti isti
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    //pokazivac + duljina
    const auto*  src    = reinterpret_cast<const unsigned char*>(encrypted.constData());
    const size_t srcLen = static_cast<size_t>(encrypted.size());

    //koliko zelimo rezervirat
    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, src, srcLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    //desifriramo
    QByteArray plaintext(static_cast<int>(outLen), '\0');
    if (EVP_PKEY_decrypt(ctx,
                         reinterpret_cast<unsigned char*>(plaintext.data()),
                         &outLen, src, srcLen) <= 0) {
        qWarning() << "[Crypto] decryptUsersJson: RSA decrypt failed:" << opensslError();
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    //cistimo i resizeamo
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

// ============================================================================
// NOVO: hibridna (envelope) enkripcija — EVP_Seal / EVP_Open
// AES-256-CBC sifrira podatke, RSA javni kljuc omota nasumicni AES kljuc.
// Format: [4B ekl big-endian][ek (ekl B)][IV (16 B)][AES ciphertext]
// ============================================================================

QByteArray CryptoManager::sealHybrid(const QByteArray& data, const QString& publicKeyPath) {
    // 1. ucitaj javni kljuc
    EVP_PKEY* pkey = rsaLoadPublicKey(publicKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] sealHybrid: cannot load public key:" << opensslError();
        return {};
    }

    // 2. cipher kontekst
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return {};
    }

    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    const int ivLen = EVP_CIPHER_iv_length(cipher);   // 16

    // 3. buffer za RSA-omotani AES kljuc (max = velicina RSA kljuca) + IV
    QByteArray ek(EVP_PKEY_size(pkey), '\0');
    unsigned char* ekPtr = reinterpret_cast<unsigned char*>(ek.data());
    unsigned char iv[EVP_MAX_IV_LENGTH];
    int ekl = 0;

    // 4. SealInit: OpenSSL sam generira nasumicni AES kljuc, RSA ga sifrira u ek, puni iv
    if (EVP_SealInit(ctx, cipher, &ekPtr, &ekl, iv, &pkey, 1) != 1) {
        qWarning() << "[Crypto] sealHybrid: EVP_SealInit failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    // 5. sifriraj podatke
    QByteArray ciphertext(data.size() + EVP_CIPHER_block_size(cipher), '\0');
    int outLen = 0;
    if (EVP_SealUpdate(ctx,
                       reinterpret_cast<unsigned char*>(ciphertext.data()), &outLen,
                       reinterpret_cast<const unsigned char*>(data.constData()),
                       static_cast<int>(data.size())) != 1) {
        qWarning() << "[Crypto] sealHybrid: EVP_SealUpdate failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    int finalLen = 0;
    if (EVP_SealFinal(ctx,
                      reinterpret_cast<unsigned char*>(ciphertext.data()) + outLen,
                      &finalLen) != 1) {
        qWarning() << "[Crypto] sealHybrid: EVP_SealFinal failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    ciphertext.resize(outLen + finalLen);

    // 6. sloziti okvir: [4B ekl][ek][IV][ciphertext] — duljinu pisemo rucno (big-endian)
    QByteArray out;
    out.append(static_cast<char>((ekl >> 24) & 0xFF));
    out.append(static_cast<char>((ekl >> 16) & 0xFF));
    out.append(static_cast<char>((ekl >>  8) & 0xFF));
    out.append(static_cast<char>( ekl        & 0xFF));
    out.append(ek.constData(), ekl);
    out.append(reinterpret_cast<const char*>(iv), ivLen);
    out.append(ciphertext);

    // 7. ciscenje
    EVP_CIPHER_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    qInfo() << "[Crypto] sealHybrid: wrapped key" << ekl << "B, IV" << ivLen
            << "B, ciphertext" << ciphertext.size() << "B → total" << out.size() << "B";
    return out;
}

QByteArray CryptoManager::openHybrid(const QByteArray& sealed, const QString& privateKeyPath) {
    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    const int ivLen = EVP_CIPHER_iv_length(cipher);   // 16

    // 1. rasclani okvir: [4B ekl][ek][IV][ciphertext]
    if (sealed.size() < 4) {
        qWarning() << "[Crypto] openHybrid: sealed too short (no length header)";
        return {};
    }
    const unsigned char* p = reinterpret_cast<const unsigned char*>(sealed.constData());
    const int ekl = (static_cast<int>(p[0]) << 24) | (static_cast<int>(p[1]) << 16) |
                    (static_cast<int>(p[2]) <<  8) |  static_cast<int>(p[3]);

    if (ekl <= 0 || sealed.size() < static_cast<qsizetype>(4 + ekl + ivLen)) {
        qWarning() << "[Crypto] openHybrid: sealed too short or bad ekl:" << ekl;
        return {};
    }

    const unsigned char* ek = p + 4;
    const unsigned char* iv = p + 4 + ekl;
    const unsigned char* ct = p + 4 + ekl + ivLen;
    const int ctLen = static_cast<int>(sealed.size()) - (4 + ekl + ivLen);

    // 2. ucitaj privatni kljuc
    EVP_PKEY* pkey = rsaLoadPrivateKey(privateKeyPath);
    if (!pkey) {
        qWarning() << "[Crypto] openHybrid: cannot load private key:" << opensslError();
        return {};
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return {};
    }

    // 3. OpenInit: RSA odmota AES kljuc, zatim AES desifrira podatke
    if (EVP_OpenInit(ctx, cipher, ek, ekl, iv, pkey) != 1) {
        qWarning() << "[Crypto] openHybrid: EVP_OpenInit failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }

    QByteArray plaintext(ctLen + EVP_CIPHER_block_size(cipher), '\0');
    int outLen = 0;
    if (EVP_OpenUpdate(ctx,
                       reinterpret_cast<unsigned char*>(plaintext.data()), &outLen,
                       ct, ctLen) != 1) {
        qWarning() << "[Crypto] openHybrid: EVP_OpenUpdate failed:" << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    int finalLen = 0;
    if (EVP_OpenFinal(ctx,
                      reinterpret_cast<unsigned char*>(plaintext.data()) + outLen,
                      &finalLen) != 1) {
        qWarning() << "[Crypto] openHybrid: EVP_OpenFinal failed (wrong key or corrupted data):"
                   << opensslError();
        EVP_CIPHER_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    plaintext.resize(outLen + finalLen);

    EVP_CIPHER_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return plaintext;
}

QByteArray CryptoManager::encryptUsersHybrid(const QString& publicKeyPath) {
    // Ista serijalizacija kao encryptUsersJson (samo username + role, NE lozinka),
    // ali BEZ 245-bajtne provjere — hibridna enkripcija nema RSA limit velicine.
    const std::vector<User> users = DatabaseManager::instance().getAllUsers();

    QJsonArray arr;
    for (const auto& u : users) {
        QJsonObject obj;
        obj["username"] = QString::fromStdString(u.getUsername());
        obj["role"]     = QString::fromStdString(u.getRoleAsString());
        arr.append(obj);
    }
    const QByteArray json = QJsonDocument(arr).toJson(QJsonDocument::Compact);

    return sealHybrid(json, publicKeyPath);
}

QString CryptoManager::decryptUsersHybrid(const QByteArray& sealed,
                                          const QString& privateKeyPath) {
    return QString::fromUtf8(openHybrid(sealed, privateKeyPath));
}
