#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>


class CryptoManager {
public:

    static constexpr const char* PEPPER = "WAREHOUSE_PEPPER_2026";




    static QString hashPlain(const QString& password);


    static QString hashWithSalt(const QString& password, const QString& salt);

    static QString deriveSalt(const QString& username);


    static const QStringList& candidatePeppers();



    static QString hashWithSaltAndSpecificPepper(const QString& password,
                                                 const QString& salt,
                                                 const QString& pepper);


    static bool verifyPasswordBruteForcePepper(const QString& password,
                                               const QString& salt,
                                               const QString& storedHash);




    static QByteArray encryptAES(const QByteArray& data, const QString& password);


    static QByteArray decryptAES(const QByteArray& encrypted, const QString& password);




    static bool generateRSAKeyPair(const QString& privateKeyPath,
                                   const QString& publicKeyPath,
                                   int            bits = 2048);


    static QByteArray encryptRSA(const QString& plaintext, const QString& publicKeyPath);


    static QString decryptRSA(const QByteArray& ciphertext, const QString& privateKeyPath);



    static QByteArray encryptUsersJson(const QString& publicKeyPath);

    static QString decryptUsersJson(const QByteArray& encrypted,
                                    const QString&    privateKeyPath);

    // ===================== NOVO: hibridna (envelope) enkripcija =====================
    // Nasumicni AES kljuc sifrira podatke, RSA javni kljuc "omota" AES kljuc.
    // Koristi OpenSSL EVP_Seal/EVP_Open. Nema RSA ogranicenja velicine.
    static QByteArray sealHybrid(const QByteArray& data, const QString& publicKeyPath);
    static QByteArray openHybrid(const QByteArray& sealed, const QString& privateKeyPath);

    // Izvoz korisnika preko hibridne enkripcije (bez 245-bajtnog limita).
    static QByteArray encryptUsersHybrid(const QString& publicKeyPath);
    static QString    decryptUsersHybrid(const QByteArray& sealed,
                                         const QString& privateKeyPath);
    // ===============================================================================




    static QByteArray signData(const QByteArray& data, const QString& privateKeyPath);


    static bool verifySignature(const QByteArray& data,
                                const QByteArray& signature,
                                const QString&    publicKeyPath);

private:

    static constexpr int IV_SIZE  = 16;
    static constexpr int KEY_SIZE = 32;
    static void deriveKey(const QString& password, unsigned char* key32);
};
