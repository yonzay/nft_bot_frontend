#include "Auth.h"
#include "Action.h"
#include "Chronicle.h"
#include "Cryptography.h"
#include "Http_Request.h"
#include "Environment.h"
#include "Utility.h"
#include "Module.h"

const char* Auth::host = "license-service-1418865070.us-east-1.elb.amazonaws.com";
const char* Auth::port = "80";

atomic<bool> Auth::auto_login = false;
string Auth::current_session;
string Auth::discord_username;
atomic<int> Auth::nft_iteration;
string Auth::nft_image_url;
string Auth::quote;
string Auth::email_address;
int64_t Auth::renewal_date;
int64_t Auth::last_login;
string Auth::wallet_address;
atomic<bool> Auth::is_logged_in = false;
string Auth::hyper_auth_cert;

void Auth::login() {
	stringstream json;
    vector<string> nonce;
    ptree base;
    ptree encrypted;
    ptree response;
    nonce = Cryptography::get_nonce(true);
    base.put("encrypted_hyper_license_key", Auth::hyper_auth_cert);
    encrypted.put("session", Auth::current_session);
    encrypted.put("nonce", nonce[0]);
    base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
    response = Http_Request::send(verb::post, Auth::host, Auth::port, ("/api/v4/user/verify_license?token=" + nonce[1]).c_str(), Utility::format_json(base).c_str(), "text/plain");
    if (response.size() == 0) {
        Chronicle::print(true, type::error, false, "Failed to send request, check your internet/client's connection closing in 5 seconds...");
        boost::this_thread::sleep_for(boost::chrono::seconds(10));
        Environment::thread_safe_function[9].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
        return;
    } else {
        if (!response.get<bool>("success")) {
            Chronicle::print(true, type::error, false, response.get<string>("reason"));
            Chronicle::print(true, type::warning, false, "Closing in 5 seconds...");
            boost::this_thread::sleep_for(boost::chrono::seconds(10));
            Environment::thread_safe_function[9].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
            return;
        } else if (response.get<bool>("success")) {
            Auth::discord_username = response.get<string>("discord_tag");
            Chronicle::print(true, type::message, false , "Successfully signed in as [" + Auth::discord_username + "].");
            Auth::nft_iteration = response.get<int>("nft_iteration");
            Auth::nft_image_url = response.get<string>("nft_image_url");
            Auth::email_address = response.get<string>("email_address");
            Auth::renewal_date = response.get_optional<int64_t>("renewal_date").get_value_or(0);
            Auth::last_login = response.get_optional<int64_t>("last_login").get_value_or(0);
            Auth::wallet_address = response.get<string>("wallet_address");
            Auth::is_logged_in = true;
            boost::filesystem::create_directory(Module::persistent_path + "UserData");
            boost::filesystem::create_directory(Module::persistent_path + "UserData/Transactions");
            boost::filesystem::create_directory(Module::persistent_path + "UserData/Analytics");
            boost::filesystem::create_directory(Module::persistent_path + "UserData/WalletGroups");
            boost::filesystem::create_directory(Module::persistent_path + "UserData/ContractTasks");
            boost::filesystem::create_directory(Module::persistent_path + "UserData/SniperTasks");
            int iterator = 0;
            BOOST_FOREACH(ptree::value_type & instance, response.get_child("salt")) {
                assert(instance.first.empty());
                Cryptography::global_kdf_salt[iterator] = instance.second.get_value<uint8_t>();
                iterator++;
            }
            if (Utility::file_exists(Module::persistent_path + "has_derived.json")) {
                Environment::thread_safe_function[4].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
                Chronicle::print(true, type::message, false, "Cryptographic funds are encrypted, you are signed in but will not be able to access preexisting funds on this computer without the correct passphrase. Delete has_derived.json and restart to permanently lose access to preexisting funds and to construct a new passphrase for future funds.");
            } else {
                Environment::thread_safe_function[3].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
                Chronicle::print(true, type::message, false, "Enter and confirm a passphrase to store cryptographic funds on this device, forgetting this passphrase will result in funds being permanently loss relative to this application.");
            }
        }
    }
}

void Auth::load_hyper_auth_cert() {
	ifstream file(Module::persistent_path + "hyper_auth_cert.crt");
	getline(file, Auth::hyper_auth_cert);
	if (!Auth::hyper_auth_cert.empty()) {
		Auth::auto_login = true;
	} else {
		Chronicle::print(false, type::warning, false, "Couldn't locate any preexisting authentication certificate, a one time login will be required.");
	}
}