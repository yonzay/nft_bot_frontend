#pragma once
#include <botan/x509_key.h>
#include <botan/auto_rng.h>
#include <botan/base64.h>
#include <botan/pubkey.h>
#include <openssl/hmac.h>
#include <botan/pkcs8.h>
#include <botan/aes.h>
#include <botan/pipe.h>
#include <botan/sha2_32.h>
#include <botan/pbkdf2.h>
#include <botan/hmac.h>
#include <botan/cipher_mode.h>
#include <botan/filters.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Cryptography {
    static const char* host;
    static const char* port;
    static string derived_key;
    static const vector<string> rsa_public_key;
public:
    static string rsa_encrypt(const string plain_text);
    static string hmac(const string key, const string message);
    static string aes_encrypt(const string plain_text);
    static string aes_decrypt(const string plain_text);
    static uint8_t global_kdf_salt[32];
    static void derive_key(const string passphrase);
    static vector<string> get_nonce(bool threaded);
};