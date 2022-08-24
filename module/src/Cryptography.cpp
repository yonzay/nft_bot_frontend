#include "Cryptography.h"
#include "Http_Request.h"
#include "Chronicle.h"
#include "Utility.h"

const char* Cryptography::host = "license-service-1418865070.us-east-1.elb.amazonaws.com";
const char* Cryptography::port = "80";

const vector<string> Cryptography::rsa_public_key = { "-----BEGIN PUBLIC KEY-----\n",
"MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA8L383qDYGT+9GpiS91fE\n",
"Ktx1RudtD6eCmyqTKG7k/GpYQZb8tNj08U6ryN69KfHecvBxDNG5gnnZn6mTrRWU\n",
"xFRPYUHIwyL3LaP/tl53QBAghcJBp/vkuzw31r7wNGLgJRgHwGSCy/ikwO/OWs13\n",
"Zxni08566i5AbqXNBklfi+s91WAcrA4F75pXnKO2Wkjxl0DCKiYoBKsjjQy9jHTv\n",
"vRTgAANFGqX5WhYwA5/8CBCkyCkcXI2fpTjArgIGthB7XntavI9OIKiMTYdWo/Ir\n",
"K/gmOJ/ut10fuJ+pKRqrHWJAOhkCYxNL2hncocYImgLDLXs5qLzovQjJRFvuTezC\n",
"s2M4wJz6Wae0tzIJy4yQTsVnC8ZdNEpR/jB1CzqukLHA/GPIQ2jxo36uFdks4Too\n",
"K5jrnFieIRsif3WS4qjjITvKl9kmFxxiuuXYqlxAkZGHtVU522smH14UDUBIrmNf\n",
"ne7zmh9iDr20bZ5+M1QGttZlIbSc/TKrY6ZUISY+fu746HNDXNLar7kXhlAsn1gx\n",
"VvplpEmtP0/OIpCrcNWMJNSN5+yALWA9AUn00epZ+GlY8gHaWniIoHnfl4w879Xj\n",
"lOuaomBo4wlFzvCStt0W3Msptd++RKfIX5BKQKHdGyUaHcT7qIjyAYlKQAqJcjnK\n",
"Z0VPeafqEy3HL72bQu6TwH8CAwEAAQ==\n",
"-----END PUBLIC KEY-----" };

string Cryptography::derived_key;

uint8_t Cryptography::global_kdf_salt[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

string Cryptography::rsa_encrypt(const string plain_text) {
    string key;
    unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::AutoSeeded_RNG);
    vector<uint8_t> pt(plain_text.data(), plain_text.data() + plain_text.length());
    for (int x = 0; x < Cryptography::rsa_public_key.size(); x++) { key += Cryptography::rsa_public_key[x]; }
    vector<uint8_t> keyBytes(key.data(), key.data() + key.length());
    unique_ptr<Botan::Public_Key> kp(Botan::X509::load_key(keyBytes));
    Botan::PK_Encryptor_EME enc(*kp, *rng.get(), "PKCS1v15");
    vector<uint8_t> ct = enc.encrypt(pt, *rng.get());
    return Botan::base64_encode(ct);
}

string Cryptography::hmac(string key, string message) {
    unsigned char hash[32];
    HMAC_CTX* hmac = HMAC_CTX_new();
    HMAC_CTX_reset(hmac);
    HMAC_Init_ex(hmac, &key[0], key.length(), EVP_sha256(), NULL);
    HMAC_Update(hmac, (unsigned char*)&message[0], message.length());
    unsigned int len = 32;
    HMAC_Final(hmac, hash, &len);
    HMAC_CTX_free(hmac);
    stringstream result;
    result << std::hex << std::setfill('0');
    for (int i = 0; i < len; i++) { result << std::hex << std::setw(2) << (unsigned int)hash[i]; }
    return (result.str());
}

string Cryptography::aes_encrypt(const string plain_text) {
    Botan::Pipe encryptor(Botan::get_cipher("AES-256/CBC", Botan::SymmetricKey(derived_key), Botan::ENCRYPTION), new Botan::Hex_Encoder);
    encryptor.process_msg(plain_text);
    return encryptor.read_all_as_string(0);
}

string Cryptography::aes_decrypt(const string plain_text) {
    Botan::Pipe decryptor(new Botan::Hex_Decoder, Botan::get_cipher("AES-256/CBC", Botan::SymmetricKey(derived_key), Botan::DECRYPTION));
    decryptor.process_msg(plain_text);
    return decryptor.read_all_as_string(0);
}

void Cryptography::derive_key(const string passphrase) {
    Cryptography::derived_key = Botan::PKCS5_PBKDF2(new Botan::HMAC(new Botan::SHA_256)).derive_key(32, passphrase, Cryptography::global_kdf_salt, 32, 10000).to_string();
}

vector<string> Cryptography::get_nonce(bool threaded) {
    vector<string> result;
    ptree response;
    response = Http_Request::send(verb::get, Cryptography::host, Cryptography::port, "/api/v4/user/token", NULL, NULL);
    if (response.size() == 0) {
        Chronicle::print(threaded, type::error, false, "Failed to obtain nonce, attempt to perform action will fail.");
        result.push_back("");
        result.push_back("");
        return result;
    } else {
        result.push_back(Cryptography::hmac(response.get_optional<string>("token").get_value_or(""), response.get_optional<string>("time").get_value_or("")));
        result.push_back(response.get_optional<string>("token").get_value_or(""));
        Chronicle::print(threaded, type::message, false, "Obtained nonce, attempting action...");
        return result;
    }
}