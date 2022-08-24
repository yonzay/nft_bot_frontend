#include "Action.h"
#include "Auth.h"
#include "Http_Request.h"
#include "Cryptography.h"
#include "Contract_Task.h"
#include "Utility.h"
#include "Sniper_Task.h"
#include "Instance_Summary.h"
#include "Analytics.h"
#include "Environment.h"
#include "Module.h"

const char* Action::host = "blockchain-service-1859773842.us-east-1.elb.amazonaws.com";
const char* Action::port = "80";

void Action::login(const Napi::CallbackInfo& info) {
    stringstream json;
    vector<string> nonce;
    ptree container;
    ptree base;
    ptree encrypted;
    ptree response;
    nonce = Cryptography::get_nonce(false);
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, container);
        container.get<string>("Hyper_License_Key");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to sign in.");
        return;
    }
    encrypted.put("hyper_license_key", container.get<string>("Hyper_License_Key"));
    encrypted.put("session", Auth::current_session);
    encrypted.put("nonce", nonce[0]);
    base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
    response = Http_Request::send(verb::post, "license-service-1418865070.us-east-1.elb.amazonaws.com", Action::port, ("/api/v4/user/verify_license?token=" + nonce[1]).c_str(), Utility::format_json(base).c_str(), "text/plain");
    if (response.size() == 0) {
        Chronicle::print(false, type::error, false, "Failed to send request, check your internet/client's connection and try again.");
        return;
    } else {
        if (!response.get<bool>("success")) {
            Chronicle::print(false, type::error, false, response.get<string>("reason"));
            return;
        } else if (response.get<bool>("success")) {
            Environment::function_reference[5].Call({ Napi::Boolean::New(Environment::context->Global().Env(), true) });
            Environment::function_reference[10].Call({});
            ofstream hyper_cert;
            ptree cert_info;
            cert_info.put("hyper_license_key", container.get<string>("Hyper_License_Key"));
            hyper_cert.open(Module::persistent_path + "hyper_auth_cert.crt");
            hyper_cert << Cryptography::rsa_encrypt(Utility::format_json(cert_info));
            hyper_cert.close();
            Auth::discord_username = response.get<string>("discord_tag");
            Chronicle::print(false, type::message, false , "Successfully signed in as [" + Auth::discord_username + "].");
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
                Environment::function_reference[4].Call({});
                Chronicle::print(false, type::message, false, "Cryptographic funds are encrypted, you are signed in but will not be able to access preexisting funds on this computer without the correct passphrase. Delete has_derived.json and restart to permanently lose access to preexisting funds and to construct a new passphrase for future funds.");
            } else {
                Environment::function_reference[3].Call({});
                Chronicle::print(false, type::message, false, "Enter and confirm a passphrase to store cryptographic funds on this device, forgetting this passphrase will result in funds being permanently loss relative to this application.");
            }
        }
    }
}

void Action::key_derivation(const Napi::CallbackInfo& info) {
    stringstream json;
    ptree container;
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, container);
        if (container.get<string>("Passphrase").empty()) { Chronicle::print(false, type::error, false, "[Passphrase] cannot be empty."); return; }
        container.get<string>("Confirm_Passphrase");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to derive key.");
        return;
    }
    if (container.get<string>("Passphrase") != container.get<string>("Confirm_Passphrase")) {
        Chronicle::print(false, type::error, false, "Failed to confirm passphrase, please make sure both entries are correct.");
        return;
    }
    Cryptography::derive_key(container.get<string>("Passphrase"));
    ptree has_derived;
    has_derived.put("DO_NOT_TAMPER_WITH_THIS_DATA", Cryptography::aes_encrypt("correct_key"));
    write_json(Module::persistent_path + "has_derived.json", has_derived);
    Chronicle::print(false, type::message, false, "Passphrase has been set.");
    Chronicle::print(false, type::warning, false, "Do not tamper with has_derived.json or any files concerning this application, failure to do this may result in permanent loss of cryptographic funds.");
    Environment::function_reference[6].Call({ Napi::Boolean::New(Environment::context->Global().Env(), true) });
    Environment::function_reference[11].Call({});
    Chronicle::print(false, type::message, false, "Please wait until all assets/resources are loaded...");
    Action::load_user_data();
}

void Action::decrypt(const Napi::CallbackInfo& info) {
    stringstream json;
    ptree container;
    ptree has_derived;
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, container);
        container.get<string>("Passphrase");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to decrypt.");
        return;
    }
    Cryptography::derive_key(container.get<string>("Passphrase"));
    read_json(Module::persistent_path + "has_derived.json", has_derived);
    try {
        if (Cryptography::aes_decrypt(has_derived.get<string>("DO_NOT_TAMPER_WITH_THIS_DATA")) != "correct_key") {
            Environment::function_reference[9].Call({});
            return;
        }
    } catch (exception const& e) {
        Environment::function_reference[9].Call({});
        return;
    }
    Chronicle::print(false, type::message, false, "Correct key derived.");
    Environment::function_reference[7].Call({ Napi::Boolean::New(Environment::context->Global().Env(), true) });
    Environment::function_reference[12].Call({});
    Chronicle::print(false, type::message, false, "Please wait until all assets/resources are loaded...");
    Action::load_user_data();
}

void Action::load_user_data() {
    string encrypted_wallet_groups;
    string proxy_line;
    stringstream decrypted_wallet_groups_container;
    Environment::title_bar_update_content("#3FFF69", "Online");
    Environment::main_update_subscription_info();
    Environment::main_update_dashboard_card();
    try {
        Chronicle::print(false, type::message, false, "Loading [Transaction](s)...");
        read_json(Module::persistent_path + "UserData/Transactions/transaction_library.json", Module::transaction_history);
        Environment::main_update_transaction_history(false, false);
    } catch (exception const& e) {
        Chronicle::print(false, type::warning, false, "Could not find any preexisting [Transaction](s), a new record will be created to keep track of further [Transaction](s).");
    }
    if (Module::transaction_history.get_child_optional("Transaction_Library") != boost::none) {
        if (Module::transaction_history.get_child("Transaction_Library").size() > 0) {
            BOOST_FOREACH(ptree::value_type & instance, Module::transaction_history.get_child("Transaction_Library")) {
                assert(instance.first.empty());
                instance.second.get<string>("Blockchain");
                instance.second.get<string>("Transaction_Hash");
                if (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - instance.second.get<int64_t>("Date_Initiated") < 86400000) {
                    Transaction transaction_made_today;
                    transaction_made_today.blockchain = instance.second.get<string>("Blockchain");
                    transaction_made_today.transaction_hash = instance.second.get<string>("Transaction_Hash");
                    transaction_made_today.date_initiated = instance.second.get<int64_t>("Date_Initiated");
                    Module::todays_transactions.push_back(transaction_made_today);
                }
            }
            Environment::main_update_todays_transactions(false);
            Chronicle::print(false, type::message, false, "Loaded [" + to_string(Module::transaction_history.get_child("Transaction_Library").size()) + "] [Transaction](s).");
        }
    }
    try {
        Chronicle::print(false, type::message, false, "Loading analytics...");
        read_json(Module::persistent_path + "UserData/Analytics/analytics.json", Analytics::analytics_container);
    } catch (exception const& e) {
        Chronicle::print(false, type::warning, false , "Could not find any preexisting records concerning analytics, a new record will be created to keep track of further activity.");
    }
    Analytics::total_transactions = Module::transaction_history.get_child_optional("Transaction_Library").get_value_or(Module::transaction_history).size();
    Analytics::total_ethereum_spent = Analytics::analytics_container.get_optional<float>("Analytics.Ethereum.Total_Ethereum_Spent").get_value_or(0);
    Analytics::total_solana_spent = Analytics::analytics_container.get_optional<float>("Analytics.Solana.Total_Solana_Spent").get_value_or(0);
    Analytics::from_to_now = Analytics::analytics_container.get_optional<int64_t>("Analytics.From_To_Now").get_value_or(0);
    if (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - Analytics::from_to_now > 86400000) {
        Analytics::analytics_container.put("Analytics.Ethereum.Total_Ethereum_Spent_Today", 0);
        Analytics::analytics_container.put("Analytics.Solana.Total_Solana_Spent_Today", 0);
        Analytics::analytics_container.put("Analytics.From_To_Now", duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
        write_json(Module::persistent_path + "UserData/Analytics/analytics.json", Analytics::analytics_container);
        Analytics::from_to_now = Analytics::analytics_container.get<int64_t>("Analytics.From_To_Now");
    }
    Analytics::total_ethereum_spent_today = Analytics::analytics_container.get_optional<float>("Analytics.Ethereum.Total_Ethereum_Spent_Today").get_value_or(0);
    Analytics::total_solana_spent_today = Analytics::analytics_container.get_optional<float>("Analytics.Solana.Total_Solana_Spent_Today").get_value_or(0);
    Analytics::total_usd_spent = (Analytics::total_ethereum_spent * Instance_Summary::ethereum_value) + (Analytics::total_solana_spent * Instance_Summary::solana_value);
    Analytics::total_usd_spent_today = (Analytics::total_ethereum_spent_today * Instance_Summary::ethereum_value) + (Analytics::total_solana_spent_today * Instance_Summary::solana_value);
    Environment::main_update_analytics(false);
    ifstream wallet_group_file(Module::persistent_path + "UserData/WalletGroups/wallet_groups.json");
    std::getline(wallet_group_file, encrypted_wallet_groups);
    if (!encrypted_wallet_groups.empty()) { decrypted_wallet_groups_container << Cryptography::aes_decrypt(encrypted_wallet_groups); }
    try {
        Chronicle::print(false, type::message, false, "Loading [Wallet-Group](s)...");
        read_json(decrypted_wallet_groups_container, Module::wallet_groups_container);
    } catch (exception const& e) {
        Chronicle::print(false, type::warning, false, "Could not find any preexisting [Wallet-Group](s), a new record will be created to keep track of further [Wallet-Group](s).");
    }
    if (Module::wallet_groups_container.get_child_optional("Wallet_Groups") != boost::none) {
        if (Module::wallet_groups_container.get_child("Wallet_Groups").size() > 0) {
            BOOST_FOREACH(ptree::value_type & instance, Module::wallet_groups_container.get_child("Wallet_Groups")) {
                assert(instance.first.empty());
                Wallet_Group wallet_group;
                wallet_group.name = instance.second.get<string>("Name");
                wallet_group.blockchain = instance.second.get<string>("Blockchain");
                BOOST_FOREACH(ptree::value_type & instance, instance.second.get_child("Private_Keys")) {
                    assert(instance.first.empty());
                    wallet_group.private_keys.push_back(instance.second.data());
                }
                Module::wallet_groups.push_back(wallet_group);
            }
            BOOST_FOREACH(ptree::value_type & instance, Module::wallet_groups_container.get_child("Metadata")) {
                assert(instance.first.empty());
                Wallet_Group::metadata info;
                info.name = instance.second.get<string>("Name");
                info.value = instance.second.get<float>("Value");
                info.market_value = instance.second.get<int>("Market_Value");
                info.acronym = instance.second.get<string>("Acronym");
                Wallet_Group::info.push_back(info);
            }
            Environment::main_update_wallet_groups();
            Chronicle::print(false, type::message, false, "Loaded [" + to_string(Module::wallet_groups.size()) + "] [Wallet-Group](s).");
        }
    }
    try {
        Chronicle::print(false, type::message, false, "Loading settings...");
        read_json(Module::persistent_path + "settings.json", Settings::settings_container);
    } catch (exception const& e) {
        Chronicle::print(false, type::warning, false, "Could not find any preexisting setting records, a new record will be created to store settings.");
    }
    Settings::ethereum_https_provider_api_url = Settings::settings_container.get_optional<string>("Settings.Ethereum.HTTPS_Provider_API_URL").get_value_or("");
    Settings::ethereum_websocket_provider_api_url = Settings::settings_container.get_optional<string>("Settings.Ethereum.WebSocket_Provider_API_URL").get_value_or("");
    if (Settings::settings_container.get_child_optional("Settings.Solana.RPC_URLS") != boost::none) {
        if (Settings::settings_container.get_child("Settings.Solana.RPC_URLS").size() > 0) {
            BOOST_FOREACH(ptree::value_type & instance, Settings::settings_container.get_child("Settings.Solana.RPC_URLS")) {
                assert(instance.first.empty());
                Settings::solana_rpc_urls.push_back(instance.second.get_value<string>());
            }
        }
    }
    Settings::ethereum_chain_id = Settings::settings_container.get_optional<int>("Settings.Ethereum.Chain_ID").get_value_or(1);
    Settings::use_flashbots = Settings::settings_container.get_optional<bool>("Settings.Ethereum.Use_Flashbots").get_value_or(Settings::use_flashbots);
    Settings::flashbots_signer = Settings::settings_container.get_optional<string>("Settings.Ethereum.Flashbots_Signer").get_value_or("");
    Settings::discord_webhook = Settings::settings_container.get_optional<string>("Settings.Discord_Webhook").get_value_or("");
    Settings::path_to_proxies = Settings::settings_container.get_optional<string>("Settings.Path_To_Proxies").get_value_or("");
    ifstream proxies_file(Settings::path_to_proxies);
    while (std::getline(proxies_file, proxy_line)) { Settings::loaded_proxies.push_back(proxy_line); }
    Chronicle::print(false, type::message, false, "Successfully loaded [" + to_string(Settings::loaded_proxies.size()) + "] [Proxies].");
    try {
        Chronicle::print(false, type::message, false, "Loading [Contract-Task](s)...");
        read_json(Module::persistent_path + "UserData/ContractTasks/contract_tasks.json", Module::contract_tasks_container);
    } catch (exception const& e) {
        Chronicle::print(false, type::warning, false, "Could not find any preexisting [Contract-Task](s), a new record will be created to prevent you from recreating any non-trashed preexisting [Contract-Task](s) on startup.");
    }
    if (Module::contract_tasks_container.get_child_optional("Contract_Tasks") != boost::none) {
        if (Module::contract_tasks_container.get_child("Contract_Tasks").size() > 0) {
            BOOST_FOREACH(ptree::value_type& instance, Module::contract_tasks_container.get_child("Contract_Tasks")) {
                assert(instance.first.empty());
                unique_ptr<Contract_Task> contract_task_;
                contract_task_.reset(new Contract_Task());
                while (true) {
                    bool unique_id = true;
                    int id = rand() % (999 - 100) + 100;
                    std::srand(time(0));
                    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
                        if (id == Module::contract_tasks_[x]->task_id) {
                            unique_id = false;
                        }
                    }
                    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
                        if (id == Module::sniper_tasks_[x]->task_id) {
                            unique_id = false;
                        }
                    }
                    if (unique_id) { contract_task_->task_id = id; break; }
                }
                contract_task_->blockchain = instance.second.get<string>("Blockchain");
                for (int x = 0; x < Module::wallet_groups.size(); x++) {
                    if (Module::wallet_groups[x].name == instance.second.get<string>("Wallet_Group")) {
                        contract_task_->wallet_group = Module::wallet_groups[x];
                        break;
                    }
                }
                if (contract_task_->blockchain == "Ethereum") {
                    contract_task_->is_polling = false;
                    contract_task_->ethereum_contract_address = instance.second.get<string>("Ethereum.Contract_Address");
                    contract_task_->ethereum_value = instance.second.get<string>("Ethereum.Value");
                    contract_task_->ethereum_gas_price = instance.second.get<int>("Ethereum.Gas_Price");
                    contract_task_->ethereum_gas_limit = instance.second.get<int>("Ethereum.Gas_Limit");
                    contract_task_->ethereum_priority_fee = instance.second.get<int>("Ethereum.Priority_Fee");
                    contract_task_->ethereum_function = instance.second.get<string>("Ethereum.Function");
                    contract_task_->ethereum_amount_of_txns = 0;
                    ptree arguments;
                    BOOST_FOREACH(ptree::value_type & instance, instance.second.get_child("Ethereum.Call_Data")) {
                        assert(instance.first.empty());
                        ptree argument;
                        argument.put("type", instance.second.get<string>("type"));
                        argument.put("value", instance.second.get<string>("value"));
                        arguments.push_back(std::make_pair("", argument));
                    }
                    contract_task_->ethereum_call_data = arguments;
                } else if (contract_task_->blockchain == "Solana") {
                    contract_task_->solana_mode = instance.second.get<string>("Solana.Mode");
                    if (contract_task_->solana_mode == "CMV1") {
                        contract_task_->solana_candy_machine_id = instance.second.get<string>("Solana.Candy_Machine_V1.Id");
                        contract_task_->solana_candy_machine_amount = instance.second.get<int>("Solana.Candy_Machine_V1.Amount");
                    } else if (contract_task_->solana_mode == "CMV2") {
                        contract_task_->solana_candy_machine_id = instance.second.get<string>("Solana.Candy_Machine_V2.Id");
                        contract_task_->solana_candy_machine_amount = instance.second.get<int>("Solana.Candy_Machine_V2.Amount");
                    }
                }
                contract_task_->time_started = instance.second.get<int64_t>("Time_Started");
                Module::contract_tasks_.push_back(std::move(contract_task_));
                for (int x = 0; x < Contract_Task::stop_pool.size(); x++) {
                    if (Contract_Task::stop_pool[x] == 0) {
                        Contract_Task::stop_pool[x] = Module::contract_tasks_.back()->task_id;
                        break;
                    }
                }
                for (int x = 0; x < Module::contract_tasks_.back()->wallet_group.private_keys.size(); x++) {
                    boost::thread(&Contract_Task::instance, Module::contract_tasks_.back().get(), x);
                }
                Instance_Summary::total_contract_tasks++;
            }
            Environment::main_update_contract_tasks(false);
            Chronicle::print(false, type::message, false, "Loaded [" + to_string(Module::contract_tasks_.size()) + "] [Contract-Task](s).");
        }
    }
    try {
        Chronicle::print(false, type::message, false, "Loading [Sniper-Task](s)...");
        read_json(Module::persistent_path + "UserData/SniperTasks/sniper_tasks.json", Module::sniper_tasks_container);
    } catch (exception const& e) {
        Chronicle::print(false, type::warning, false, "Could not find any preexisting [Sniper-Task](s), a new record will be created to prevent you from recreating any non-trashed preexisting [Sniper-Task](s) on startup.");
    }
    if (Module::sniper_tasks_container.get_child_optional("Sniper_Tasks") != boost::none) {
        if (Module::sniper_tasks_container.get_child("Sniper_Tasks").size() > 0) {
            BOOST_FOREACH(ptree::value_type & instance, Module::sniper_tasks_container.get_child("Sniper_Tasks")) {
                assert(instance.first.empty());
                unique_ptr<Sniper_Task> sniper_task_;
                sniper_task_.reset(new Sniper_Task());
                while (true) {
                    bool unique_id = true;
                    std::srand(time(0));
                    int id = rand() % (999 - 100) + 100;
                    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
                        if (id == Module::sniper_tasks_[x]->task_id) {
                            unique_id = false;
                        }
                    }
                    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
                        if (id == Module::contract_tasks_[x]->task_id) {
                            unique_id = false;
                        }
                    }
                    if (unique_id) { sniper_task_->task_id = id; break; }
                }
                sniper_task_->blockchain = instance.second.get<string>("Blockchain");
                sniper_task_->marketplace = instance.second.get<string>("Marketplace");
                sniper_task_->marketplace_api_key = instance.second.get<string>("Marketplace_API_Key");
                sniper_task_->collection = instance.second.get<string>("Collection");
                sniper_task_->wallet_private_key = Cryptography::aes_decrypt(instance.second.get<string>("Wallet_Private_Key"));
                if (sniper_task_->blockchain == "Ethereum") {
                    sniper_task_->use_flashbots = instance.second.get<bool>("Ethereum.Use_Flashbots");
                    sniper_task_->erc_20_token_address = instance.second.get<string>("Ethereum.ERC_20_Token_Address");
                    sniper_task_->ethereum_gas_price = instance.second.get<int>("Ethereum.Gas_Price");
                    sniper_task_->ethereum_gas_limit = instance.second.get<int>("Ethereum.Gas_Limit");
                    sniper_task_->ethereum_priority_fee = instance.second.get<int>("Ethereum.Priority_Fee");
                    sniper_task_->ethereum_nonce = instance.second.get<int>("Ethereum.Nonce");
                } else if (sniper_task_->blockchain == "Solana") {

                }
                sniper_task_->maximum_value = instance.second.get<float>("Maximum_Value");
                sniper_task_->minimum_value = instance.second.get<float>("Minimum_Value");
                sniper_task_->delay = instance.second.get<int>("Delay");
                Module::sniper_tasks_.push_back(std::move(sniper_task_));
                for (int x = 0; x < Sniper_Task::stop_pool.size(); x++) {
                    if (Sniper_Task::stop_pool[x] == 0) {
                        Sniper_Task::stop_pool[x] = Module::sniper_tasks_.back()->task_id;
                        break;
                    }
                }
                boost::thread(&Sniper_Task::instance, Module::sniper_tasks_.back().get(), 0);
                Instance_Summary::total_sniper_tasks++;
            }
            Environment::main_update_sniper_tasks(false);
            Chronicle::print(false, type::message, false, "Loaded [" + to_string(Module::sniper_tasks_.size()) + "] [Sniper-Task](s).");
        }
    }
    Environment::main_update_instance_summary(false);
    Environment::function_reference[13].Call({});
    Chronicle::print(false, type::message, false, "Welcome to MoonDweller, [" + Auth::discord_username + "]. : Enterprise NFT Botting Software.");
}

Napi::String Action::view_settings(const Napi::CallbackInfo& info) {
    ptree settings_json;
    settings_json.put("Settings.Ethereum.HTTPS_Provider_API_URL", Settings::ethereum_https_provider_api_url);
    settings_json.put("Settings.Ethereum.WebSocket_Provider_API_URL", Settings::ethereum_websocket_provider_api_url);
    settings_json.put("Settings.Ethereum.Chain_ID", Settings::ethereum_chain_id);
    settings_json.put("Settings.Ethereum.Use_Flashbots", Settings::use_flashbots.load());
    settings_json.put("Settings.Ethereum.Flashbots_Signer", Settings::flashbots_signer);
    ptree rpc_urls;
    if (Settings::solana_rpc_urls.size() > 0) {
        for (int x = 0; x < Settings::solana_rpc_urls.size(); x++) {
            ptree node;
            node.put("", Settings::solana_rpc_urls[x]);
            rpc_urls.push_back(std::make_pair("", node));
        }
    } else {
        ptree node;
        node.put("", "");
        rpc_urls.push_back(std::make_pair("", node));
    }
    settings_json.put_child("Settings.Solana.RPC_URLS", rpc_urls);
    settings_json.put("Settings.Discord_Webhook", Settings::discord_webhook);
    settings_json.put("Settings.Path_To_Proxies", Settings::path_to_proxies);
    string formatted;
    string json = Utility::format_json(settings_json);
    for (int x = 0; x < json.length(); x++) {
        if (json[x] == '\\') {
            formatted += "\\\\";
        } else {
            if (json[x] == '\'') {
                formatted += "\\'";
            } else {
                formatted += json[x];
            }
        }
    }
    return Napi::String::New(Environment::context->Global().Env(), formatted);
}

void Action::set_settings(const Napi::CallbackInfo& info) {
    stringstream json;
    ptree container;
    string proxy_line;
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, container);
        container.get<string>("Settings.Ethereum.HTTPS_Provider_API_URL");
        container.get<string>("Settings.Ethereum.WebSocket_Provider_API_URL");
        container.get<int>("Settings.Ethereum.Chain_ID");
        container.get<bool>("Settings.Ethereum.Use_Flashbots");
        container.get<string>("Settings.Ethereum.Flashbots_Signer");
        container.get_child("Settings.Solana.RPC_URLS");
        container.get<string>("Settings.Discord_Webhook");
        container.get<string>("Settings.Path_To_Proxies");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted for settings.");
        return;
    }
    Settings::ethereum_https_provider_api_url = container.get<string>("Settings.Ethereum.HTTPS_Provider_API_URL");
    Settings::ethereum_websocket_provider_api_url = container.get<string>("Settings.Ethereum.WebSocket_Provider_API_URL");
    Settings::solana_rpc_urls.clear();
    if (container.get_child_optional("Settings.Solana.RPC_URLS").has_value()) {
        BOOST_FOREACH(ptree::value_type & instance, container.get_child("Settings.Solana.RPC_URLS")) {
            assert(instance.first.empty());
            Settings::solana_rpc_urls.push_back(instance.second.get_value<string>());
        }
    }
    Settings::ethereum_chain_id = container.get<int>("Settings.Ethereum.Chain_ID");
    Settings::use_flashbots = container.get<bool>("Settings.Ethereum.Use_Flashbots");
    Settings::flashbots_signer = container.get<string>("Settings.Ethereum.Flashbots_Signer");
    Settings::discord_webhook = container.get<string>("Settings.Discord_Webhook");
    Settings::path_to_proxies = container.get<string>("Settings.Path_To_Proxies");
    ifstream file(Settings::path_to_proxies);
    Settings::loaded_proxies.clear();
    while (getline(file, proxy_line)) { Settings::loaded_proxies.push_back(proxy_line); }
    Chronicle::print(false, type::message, false, "Successfully loaded [" + to_string(Settings::loaded_proxies.size()) + "] [Proxies].");
    write_json(Module::persistent_path + "settings.json", container);
    Chronicle::print(false, type::message, false, "Set desired settings.");
}

void Action::update_analytics(bool threaded) {
    Analytics::analytics_container.put("Analytics.Ethereum.Total_Ethereum_Spent", Analytics::total_ethereum_spent);
    Analytics::analytics_container.put("Analytics.Solana.Total_Solana_Spent", Analytics::total_solana_spent);
    if (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - Analytics::from_to_now > 86400000) {
        Analytics::analytics_container.put("Analytics.Ethereum.Total_Ethereum_Spent_Today", 0);
        Analytics::analytics_container.put("Analytics.Solana.Total_Solana_Spent_Today", 0);
        Analytics::analytics_container.put("Analytics.From_To_Now", duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
        write_json(Module::persistent_path + "UserData/Analytics/analytics.json", Analytics::analytics_container);
        Analytics::from_to_now = Analytics::analytics_container.get<int64_t>("Analytics.From_To_Now");
        Analytics::total_transactions = Module::transaction_history.get_child_optional("Transaction_Library").get_value_or(Module::transaction_history).size();
        Analytics::total_usd_spent = (Analytics::total_ethereum_spent * Instance_Summary::ethereum_value) + (Analytics::total_solana_spent * Instance_Summary::solana_value);
        Analytics::total_usd_spent_today = (Analytics::total_ethereum_spent_today * Instance_Summary::ethereum_value) + (Analytics::total_solana_spent_today * Instance_Summary::solana_value);
        Environment::main_update_analytics(threaded);
        Chronicle::print(threaded, type::message, false, "Updated analytics.");
        return;
    }
    Analytics::analytics_container.put("Analytics.Ethereum.Total_Ethereum_Spent_Today", Analytics::total_ethereum_spent_today);
    Analytics::analytics_container.put("Analytics.Solana.Total_Solana_Spent_Today", Analytics::total_solana_spent_today);
    write_json(Module::persistent_path + "UserData/Analytics/analytics.json", Analytics::analytics_container);
    Analytics::total_transactions = Module::transaction_history.get_child_optional("Transaction_Library").get_value_or(Module::transaction_history).size();
    Analytics::total_usd_spent = (Analytics::total_ethereum_spent * Instance_Summary::ethereum_value) + (Analytics::total_solana_spent * Instance_Summary::solana_value);
    Analytics::total_usd_spent_today = (Analytics::total_ethereum_spent_today * Instance_Summary::ethereum_value) + (Analytics::total_solana_spent_today * Instance_Summary::solana_value);
    Environment::main_update_analytics(threaded);
    Chronicle::print(threaded, type::message, false, "Updated analytics.");
}

void Action::create_contract_task(const Napi::CallbackInfo& info) {
    if (Module::contract_tasks_.size() == 100) {
        Chronicle::print(false, type::warning, false, "You cannot create over [100] [Contract-Task](s), delete one and try again.");
        return;
    }
    stringstream json;
    ptree container;
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, container);
        if (container.get<string>("Blockchain") != "Ethereum" && container.get<string>("Blockchain") != "Solana") {
            Chronicle::print(false, type::error, false, "Blockchain [" + container.get<string>("Blockchain") + "] is not supported.");
            return;
        } else if (container.get<string>("Blockchain") == "Ethereum") {
            if (container.get<string>("Ethereum.Contract_Address").empty()) { Chronicle::print(false, type::error, false, "[Contract-Address] cannot be empty."); return; }
            if (container.get<string>("Ethereum.Function").empty()) { Chronicle::print(false, type::error, false, "[Function] cannot be empty."); return; }
            if (container.get_child("Ethereum.Call_Data").empty()) { Chronicle::print(false, type::error, false, "[Call-Data] cannot be empty."); return; }
            if (!container.get<string>("Ethereum.Poll.Type").empty()) {
                if (container.get<string>("Ethereum.Poll.Type") == "Non_Flip") {
                    if (container.get<string>("Ethereum.Poll.Non_Flip.Variable_Name").empty()) { Chronicle::print(false, type::error, false, "[Variable_Name] cannot be empty."); return; }
                    container.get<int>("Ethereum.Poll.Non_Flip.Value");
                } else if (container.get<string>("Ethereum.Poll.Type") == "Flip") {
                    if (container.get<string>("Ethereum.Poll.Flip.Owner_Address").empty()) { Chronicle::print(false, type::error, false, "[Owner_Address] cannot be empty."); return; }
                    if (container.get<string>("Ethereum.Poll.Flip.Function").empty()) { Chronicle::print(false, type::error, false, "[Function] cannot be empty."); return; }
                } else {
                    Chronicle::print(false, type::error, false, "[Type] must either be [Non_Flip] or [Flip]."); return;
                }
                container.get<int>("Ethereum.Poll.Timeout");
            }
            if (container.get<string>("Ethereum.Value").empty()) { Chronicle::print(false, type::error, false, "[Value] cannot be empty."); return; }
            container.get<int>("Ethereum.Gas_Price");
            container.get<int>("Ethereum.Gas_Limit");
            container.get<int>("Ethereum.Priority_Fee");
        } else if (container.get<string>("Blockchain") == "Solana") {
            if (container.get<string>("Solana.Mode").empty()) {
                Chronicle::print(false, type::error, false, "[Id] cannot be empty."); return;
            } else if (container.get<string>("Solana.Mode") == "CMV1") {
                if (container.get<string>("Solana.Candy_Machine_V1.Id").empty()) { Chronicle::print(false, type::error, false, "[Id] cannot be empty."); return; }
                container.get<int>("Solana.Candy_Machine_V1.Amount");
            } else if (container.get<string>("Solana.Mode") == "CMV2") {
                if (container.get<string>("Solana.Candy_Machine_V2.Id").empty()) { Chronicle::print(false, type::error, false, "[Id] cannot be empty."); return; }
                container.get<int>("Solana.Candy_Machine_V2.Amount");
            } else {
                Chronicle::print(false, type::error, false, "[Mode] must either be [CMV1] or [CMV2]."); return;
            }
        }
        if (container.get<string>("Wallet_Group").empty()) { Chronicle::print(false, type::error, false, "[Wallet-Group] cannot be empty."); return; }
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to create [Contract-Task].");
        return;
    }
    unique_ptr<Contract_Task> contract_task_;
    contract_task_.reset(new Contract_Task());
    while (true) {
        bool unique_id = true;
        int id = rand() % (999 - 100) + 100;
        std::srand(time(0));
        for (int x = 0; x < Module::contract_tasks_.size(); x++) {
            if (id == Module::contract_tasks_[x]->task_id) {
                unique_id = false;
            }
        }
        for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
            if (id == Module::sniper_tasks_[x]->task_id) {
                unique_id = false;
            }
        }
        if (unique_id) { contract_task_->task_id = id; break; }
    }
    contract_task_->blockchain = container.get<string>("Blockchain");
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (container.get<string>("Wallet_Group") == Module::contract_tasks_[x]->wallet_group.name) {
            Chronicle::print(false, type::error, false, "[Contract-Task] [" + to_string(Module::contract_tasks_[x]->task_id) + "] is currently using [Wallet-Group] [" + container.get<string>("Wallet_Group") + "], choose a different one and try again.");
            return;
        }
    }
    bool found_wallet_group = false;
    for (int x = 0; x < Module::wallet_groups.size(); x++) {
        if (Module::wallet_groups[x].name == container.get<string>("Wallet_Group")) {
            found_wallet_group = true;
            if (Module::wallet_groups[x].blockchain != container.get<string>("Blockchain")) {
                Chronicle::print(false, type::error, false, "[Wallet-Group] [" + Module::wallet_groups[x].name + "] of blockchain [" + Module::wallet_groups[x].blockchain + "], isn't compatible with configured blockchain [" + container.get<string>("Blockchain") + "].");
                return;
            } else if (Module::wallet_groups[x].private_keys.size() == 0) {
                Chronicle::print(false, type::error, false, "[Wallet-Group] [" + Module::wallet_groups[x].name + "] must have 1 or more private keys to be used.");
                return;
            }
            contract_task_->wallet_group = Module::wallet_groups[x];
        }
    }
    if (!found_wallet_group) { Chronicle::print(false, type::error, false, "[Wallet-Group] [" + container.get<string>("Wallet_Group") + "] does not exist."); return; }
    if (contract_task_->blockchain == "Ethereum") {
        contract_task_->ethereum_contract_address = container.get<string>("Ethereum.Contract_Address");
        contract_task_->ethereum_function = container.get<string>("Ethereum.Function");
        if (!container.get<string>("Ethereum.Poll.Type").empty()) {
            contract_task_->is_polling = true;
        } else {
            contract_task_->is_polling = false;
        }
        contract_task_->ethereum_value = container.get<string>("Ethereum.Value");
        contract_task_->ethereum_gas_price = container.get<int>("Ethereum.Gas_Price");
        contract_task_->ethereum_gas_limit = container.get<int>("Ethereum.Gas_Limit");
        contract_task_->ethereum_priority_fee = container.get<int>("Ethereum.Priority_Fee");
        contract_task_->ethereum_amount_of_txns = 0;
        ptree arguments;
        BOOST_FOREACH(ptree::value_type & instance, container.get_child("Ethereum.Call_Data")) {
            assert(instance.first.empty());
            ptree argument;
            argument.put("type", instance.second.get<string>("type"));
            argument.put("value", instance.second.get<string>("value"));
            arguments.push_back(std::make_pair("", argument));
        }
        contract_task_->ethereum_call_data = arguments;
    } else if (contract_task_->blockchain == "Solana") {
        contract_task_->is_polling = true;
        contract_task_->solana_mode = container.get<string>("Solana.Mode");
        if (contract_task_->solana_mode == "CMV1") {
            contract_task_->solana_candy_machine_id = container.get<string>("Solana.Candy_Machine_V1.Id");
            contract_task_->solana_candy_machine_amount = container.get<int>("Solana.Candy_Machine_V1.Amount");
        } else if (contract_task_->solana_mode == "CMV2") {
            contract_task_->solana_candy_machine_id = container.get<string>("Solana.Candy_Machine_V2.Id");
            contract_task_->solana_candy_machine_amount = container.get<int>("Solana.Candy_Machine_V2.Amount");
        }
    }
    contract_task_->time_started = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    Module::contract_tasks_.push_back(std::move(contract_task_));
    ptree temp_contract_tasks_container;
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        ptree temp_contract_task;
        temp_contract_task.put("Blockchain", Module::contract_tasks_[x]->blockchain);
        temp_contract_task.put("Wallet_Group", Module::contract_tasks_[x]->wallet_group.name);
        if (Module::contract_tasks_[x]->blockchain == "Ethereum") {
            temp_contract_task.put("Ethereum.Contract_Address", Module::contract_tasks_[x]->ethereum_contract_address);
            temp_contract_task.put("Ethereum.Function", Module::contract_tasks_[x]->ethereum_function);
            temp_contract_task.put_child("Ethereum.Call_Data", Module::contract_tasks_[x]->ethereum_call_data);
            temp_contract_task.put("Ethereum.Value", Module::contract_tasks_[x]->ethereum_value);
            temp_contract_task.put("Ethereum.Gas_Price", Module::contract_tasks_[x]->ethereum_gas_price);
            temp_contract_task.put("Ethereum.Gas_Limit", Module::contract_tasks_[x]->ethereum_gas_limit);
            temp_contract_task.put("Ethereum.Priority_Fee", Module::contract_tasks_[x]->ethereum_priority_fee);
        } else if (Module::contract_tasks_[x]->blockchain == "Solana") {
            temp_contract_task.put("Solana.Mode", Module::contract_tasks_[x]->solana_mode);
            if (Module::contract_tasks_[x]->solana_mode == "CMV1") {
                temp_contract_task.put("Solana.Candy_Machine_V1.Id", Module::contract_tasks_[x]->solana_candy_machine_id);
                temp_contract_task.put("Solana.Candy_Machine_V1.Amount", Module::contract_tasks_[x]->solana_candy_machine_amount);
            } else if (Module::contract_tasks_[x]->solana_mode == "CMV2") {
                temp_contract_task.put("Solana.Candy_Machine_V2.Id", Module::contract_tasks_[x]->solana_candy_machine_id);
                temp_contract_task.put("Solana.Candy_Machine_V2.Amount", Module::contract_tasks_[x]->solana_candy_machine_amount);
            }
        }
        temp_contract_task.put("Time_Started", Module::contract_tasks_[x]->time_started);
        temp_contract_tasks_container.push_back(ptree::value_type("", temp_contract_task));
    }
    ptree save_contract_tasks;
    save_contract_tasks.put_child("Contract_Tasks", temp_contract_tasks_container);
    write_json(Module::persistent_path + "UserData/ContractTasks/contract_tasks.json", save_contract_tasks);
    if (Module::contract_tasks_.back()->blockchain == "Ethereum") {
        if (Module::contract_tasks_.back()->is_polling) {
            Instance_Summary::total_contract_tasks++;
            Environment::main_update_instance_summary(false);
            Environment::main_update_contract_tasks(false);
            if (container.get<string>("Ethereum.Poll.Type") == "Non_Flip") {
                Environment::client_create_public_variable_poll_head(Module::contract_tasks_.back()->task_id, Settings::ethereum_https_provider_api_url, Settings::ethereum_chain_id, Module::contract_tasks_.back()->ethereum_contract_address, container.get<string>("Ethereum.Poll.Non_Flip.Variable_Name"), container.get<int>("Ethereum.Poll.Non_Flip.Value"), container.get<int>("Ethereum.Poll.Timeout"));
            } else if (container.get<string>("Ethereum.Poll.Type") == "Flip") {
                Environment::client_create_pending_transaction_poll_head(Module::contract_tasks_.back()->task_id, Settings::ethereum_websocket_provider_api_url, Settings::ethereum_chain_id, Module::contract_tasks_.back()->ethereum_contract_address, container.get<string>("Ethereum.Poll.Flip.Owner_Address"), container.get<string>("Ethereum.Poll.Flip.Function"), container.get<int>("Ethereum.Poll.Timeout"));
            }
            Chronicle::print(false, type::message, false, "Successfully created [Contract-Task] [" + to_string(Module::contract_tasks_.back()->task_id) + "], this task will be unable to be modified while polling.");
            return;
        } else {
            for (int x = 0; x < Contract_Task::stop_pool.size(); x++) {
                if (Contract_Task::stop_pool[x] == 0) {
                    Contract_Task::stop_pool[x] = Module::contract_tasks_.back()->task_id;
                    break;
                }
            }
        }
    } else if (Module::contract_tasks_.back()->blockchain == "Solana") {
        Instance_Summary::total_contract_tasks++;
        Environment::main_update_instance_summary(false);
        Environment::main_update_contract_tasks(false);
        Environment::candy_machine_poll(Module::contract_tasks_.back()->task_id, Settings::solana_rpc_urls.size() > 0 ? Settings::solana_rpc_urls[0] : "", Module::contract_tasks_.back()->solana_candy_machine_id, Module::contract_tasks_.back()->solana_mode);
        Chronicle::print(false, type::message, false, "Successfully created [Contract-Task] [" + to_string(Module::contract_tasks_.back()->task_id) + "], this task will be unable to be modified while polling.");
        return;
    }
    for (int x = 0; x < Module::contract_tasks_.back()->wallet_group.private_keys.size(); x++) {
        boost::thread(&Contract_Task::instance, Module::contract_tasks_.back().get(), x);
    }
    Instance_Summary::total_contract_tasks++;
    Instance_Summary::contract_tasks_active++;
    Environment::main_update_instance_summary(false);
    Environment::main_update_contract_tasks(false);
    Chronicle::print(false, type::message, false, "Successfully created [Contract-Task] [" + to_string(Module::contract_tasks_.back()->task_id) + "].");
}

void Action::start_polling_task(const Napi::CallbackInfo& info) {
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            if (!Module::contract_tasks_[x]->is_polling) { return; }
            Module::contract_tasks_[x]->is_polling = false;
            for (int y = 0; y < Module::contract_tasks_[x]->wallet_group.private_keys.size(); y++) {
                boost::thread(&Contract_Task::instance, Module::contract_tasks_[x].get(), y);
            }
            Instance_Summary::contract_tasks_active++;
            Environment::main_update_instance_summary(false);
            Chronicle::print(false, type::message, false, "Poll returned positive for [Contract-Task] [" + to_string(Module::contract_tasks_[x]->task_id) + "], task has started and can now be modified.");
            return;
        }
    }
}

void Action::start_transaction_polling_task(const Napi::CallbackInfo& info) {
    int task_id = info[0].ToNumber().Int32Value();
    int gas_price = info[1].ToNumber().Int32Value();
    int gas_limit = info[2].ToNumber().Int32Value();
    int priority_fee = info[3].ToNumber().Int32Value();
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            Module::contract_tasks_[x]->ethereum_gas_price = gas_price;
            Module::contract_tasks_[x]->ethereum_gas_limit = gas_limit;
            Module::contract_tasks_[x]->ethereum_priority_fee = priority_fee;
            if (!Module::contract_tasks_[x]->is_polling) { return; }
            Module::contract_tasks_[x]->is_polling = false;
            for (int y = 0; y < Module::contract_tasks_[x]->wallet_group.private_keys.size(); y++) {
                boost::thread(&Contract_Task::instance, Module::contract_tasks_[x].get(), y);
            }
            Instance_Summary::contract_tasks_active++;
            Environment::main_update_instance_summary(false);
            Chronicle::print(false, type::message, false, "Poll returned positive for [Contract-Task] [" + to_string(Module::contract_tasks_[x]->task_id) + "], task has started and can now be modified.");
            return;
        }
    }
}

void Action::unlock_polling_task(const Napi::CallbackInfo& info) {
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            if (!Module::contract_tasks_[x]->is_polling) { return; }
        }
    }
    for (int x = 0; x < Contract_Task::stop_pool.size(); x++) {
        if (Contract_Task::stop_pool[x] == 0) { Contract_Task::stop_pool[x] = task_id; break; }
    }
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            Module::contract_tasks_[x]->is_polling = false;
            for (int y = 0; y < Module::contract_tasks_[x]->wallet_group.private_keys.size(); y++) {
                boost::thread(&Contract_Task::instance, Module::contract_tasks_[x].get(), y);
            }
            Chronicle::print(false, type::message, false, "Poll for [Contract-Task] [" + to_string(Module::contract_tasks_[x]->task_id) + "] has timed out or stopped, task can now be modified manually.");
            return;
        }
    }
}

void Action::create_sniper_task(const Napi::CallbackInfo& info) {
    if (Module::sniper_tasks_.size() == 100) {
        Chronicle::print(false, type::warning, false, "You cannot create over [100] [Sniper-Task](s), delete one and try again.");
        return;
    }
    stringstream json;
    ptree container;
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, container);
        if (container.get<string>("Blockchain") != "Ethereum" && container.get<string>("Blockchain") != "Solana") {
            Chronicle::print(false, type::error, false, "Blockchain [" + container.get<string>("Blockchain") + "] is not supported.");
            return;
        } else if (container.get<string>("Blockchain") == "Ethereum") {
            container.get<bool>("Ethereum.Use_Flashbots");
            if (container.get<string>("Ethereum.ERC_20_Token_Address").empty()) { Chronicle::print(false, type::error, false, "[ERC_20_Token_Address] cannot be empty."); return; }
            container.get<int>("Ethereum.Gas_Price");
            container.get<int>("Ethereum.Gas_Limit");
            container.get<int>("Ethereum.Priority_Fee");
            container.get<int>("Ethereum.Nonce");
        } else if (container.get<string>("Blockchain") == "Solana") {

        }
        if (container.get<string>("Marketplace").empty()) { 
            Chronicle::print(false, type::error, false, "[Marketplace] cannot be empty."); return;
        } else if (container.get<string>("Marketplace") != "opensea" && container.get<string>("Marketplace") != "magiceden") {
            Chronicle::print(false, type::error, false, "[Marketplace] provided isn't supported, please use either [opensea] or [magiceden]."); return;
        }
        if (container.get<string>("Collection").empty()) { Chronicle::print(false, type::error, false, "[Collection] cannot be empty."); return; }
        if (container.get<string>("Wallet_Private_Key").empty()) { Chronicle::print(false, type::error, false, "[Wallet_Private_Key] cannot be empty."); return; }
        container.get<float>("Minimum_Value");
        container.get<float>("Maximum_Value");
        container.get<int>("Delay");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to create [Sniper-Task].");
        return;
    }
    unique_ptr<Sniper_Task> sniper_task_;
    sniper_task_.reset(new Sniper_Task());
    while (true) {
        bool unique_id = true;
        int id = rand() % (999 - 100) + 100;
        std::srand(time(0));
        for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
            if (id == Module::sniper_tasks_[x]->task_id) {
                unique_id = false;
            }
        }
        for (int x = 0; x < Module::contract_tasks_.size(); x++) {
            if (id == Module::contract_tasks_[x]->task_id) {
                unique_id = false;
            }
        }
        if (unique_id) { sniper_task_->task_id = id; break; }
    }
    sniper_task_->blockchain = container.get<string>("Blockchain");
    sniper_task_->marketplace = container.get<string>("Marketplace");
    sniper_task_->marketplace_api_key = container.get<string>("Marketplace_API_Key");
    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
        if (container.get<string>("Collection") == Module::sniper_tasks_[x]->collection) {
            Chronicle::print(false, type::error, false, "[Sniper-Task] [" + to_string(Module::sniper_tasks_[x]->task_id) + "] is already set on this collection.");
            return;
        }
    }
    sniper_task_->collection = container.get<string>("Collection");
    sniper_task_->wallet_private_key = container.get<string>("Wallet_Private_Key");
    if (sniper_task_->blockchain == "Ethereum") {
        sniper_task_->use_flashbots = container.get<bool>("Ethereum.Use_Flashbots");
        sniper_task_->erc_20_token_address = container.get<string>("Ethereum.ERC_20_Token_Address");
        sniper_task_->ethereum_gas_price = container.get<int>("Ethereum.Gas_Price");
        sniper_task_->ethereum_gas_limit = container.get<int>("Ethereum.Gas_Limit");
        sniper_task_->ethereum_priority_fee = container.get<int>("Ethereum.Priority_Fee");
        sniper_task_->ethereum_nonce = container.get<int>("Ethereum.Nonce");
    } else if (sniper_task_->blockchain == "Solana") {

    }
    sniper_task_->minimum_value = container.get<float>("Minimum_Value");
    sniper_task_->maximum_value = container.get<float>("Maximum_Value");
    sniper_task_->delay = container.get<int>("Delay");
    Module::sniper_tasks_.push_back(std::move(sniper_task_));
    ptree temp_sniper_tasks_container;
    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
        ptree temp_sniper_task;
        temp_sniper_task.put("Blockchain", Module::sniper_tasks_[x]->blockchain);
        temp_sniper_task.put("Marketplace", Module::sniper_tasks_[x]->marketplace);
        temp_sniper_task.put("Marketplace_API_Key", Module::sniper_tasks_[x]->marketplace_api_key);
        temp_sniper_task.put("Collection", Module::sniper_tasks_[x]->collection);
        if (Module::sniper_tasks_[x]->blockchain == "Ethereum") {
            temp_sniper_task.put("Ethereum.Use_Flashbots", Module::sniper_tasks_[x]->use_flashbots);
            temp_sniper_task.put("Ethereum.ERC_20_Token_Address", Module::sniper_tasks_[x]->erc_20_token_address);
            temp_sniper_task.put("Ethereum.Gas_Price", Module::sniper_tasks_[x]->ethereum_gas_price);
            temp_sniper_task.put("Ethereum.Gas_Limit", Module::sniper_tasks_[x]->ethereum_gas_limit);
            temp_sniper_task.put("Ethereum.Priority_Fee", Module::sniper_tasks_[x]->ethereum_priority_fee);
            temp_sniper_task.put("Ethereum.Nonce", Module::sniper_tasks_[x]->ethereum_nonce);
        }
        temp_sniper_task.put("Wallet_Private_Key", Cryptography::aes_encrypt(Module::sniper_tasks_[x]->wallet_private_key));
        temp_sniper_task.put("Minimum_Value", Module::sniper_tasks_[x]->minimum_value);
        temp_sniper_task.put("Maximum_Value", Module::sniper_tasks_[x]->maximum_value);
        temp_sniper_task.put("Delay", Module::sniper_tasks_[x]->delay);
        temp_sniper_tasks_container.push_back(ptree::value_type("", temp_sniper_task));
    }
    ptree save_sniper_tasks;
    save_sniper_tasks.put_child("Sniper_Tasks", temp_sniper_tasks_container);
    write_json(Module::persistent_path + "UserData/SniperTasks/sniper_tasks.json", save_sniper_tasks);
    boost::thread(&Sniper_Task::instance, Module::sniper_tasks_.back().get(), 0);
    Instance_Summary::total_sniper_tasks++;
    Instance_Summary::sniper_tasks_active++;
    Environment::main_update_instance_summary(false);
    Environment::main_update_sniper_tasks(false);
    Chronicle::print(false, type::message, false, "Successfully created [Sniper-Task] [" + to_string(Module::sniper_tasks_.back()->task_id) + "].");
}

void Action::start_contract_task(const Napi::CallbackInfo& info) {
    bool already_running = true;
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            if (Module::contract_tasks_[x]->is_polling) {
                Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be started as it is polling.");
                return;
            }
            Module::contract_tasks_[x]->ethereum_amount_of_txns = 0;
        }
    }
    for (int x = 0; x < Contract_Task::delete_pool.size(); x++) {
        if (Contract_Task::delete_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be started as it is being deleted.");
            return;
        }
    }
    for (int x = 0; x < Contract_Task::stop_pool.size(); x++) {
        if (task_id == Contract_Task::stop_pool[x]) {
            Contract_Task::stop_pool[x] = 0;
            Instance_Summary::contract_tasks_active++;
            Environment::main_update_instance_summary(false);
            Chronicle::print(false, type::message, false, "Starting [Contract-Task] [" + to_string(task_id) + "]...");
            already_running = false;
        }
    }
    if (already_running) { Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be started as it is already active."); }
}

void Action::start_sniper_task(const Napi::CallbackInfo& info) {
    bool already_running = true;
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Sniper_Task::delete_pool.size(); x++) {
        if (Sniper_Task::delete_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Sniper-Task] [" + to_string(task_id) + "] cannot be started as it is being deleted.");
            return;
        }
    }
    for (int x = 0; x < Sniper_Task::stop_pool.size(); x++) {
        if (task_id == Sniper_Task::stop_pool[x]) {
            Sniper_Task::stop_pool[x] = 0;
            Instance_Summary::sniper_tasks_active++;
            Environment::main_update_instance_summary(false);
            Chronicle::print(false, type::message, false, "Starting [Sniper-Task] [" + to_string(task_id) + "]...");
            already_running = false;
        }
    }
    if (already_running) { Chronicle::print(false, type::warning, false, "[Sniper-Task] [" + to_string(task_id) + "] cannot be started as it is already active."); }
}

void Action::stop_contract_task(const Napi::CallbackInfo& info) {
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            if (Module::contract_tasks_[x]->is_polling) {
                Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be stopped as it is polling.");
                return;
            }
        }
    }
    for (int x = 0; x < Contract_Task::stop_pool.size(); x++) {
        if (Contract_Task::stop_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be stopped as it is already not active.");
            return;
        }
    }
    for (int x = 0; x < Contract_Task::delete_pool.size(); x++) {
        if (Contract_Task::delete_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be stopped as it is being deleted.");
            return;
        }
    }
    for (int x = 0; x < Contract_Task::stop_pool.size(); x++) {
        if (Contract_Task::stop_pool[x] == 0) {
            Contract_Task::stop_pool[x] = task_id;
            Instance_Summary::contract_tasks_active--;
            Environment::main_update_instance_summary(false);
            Chronicle::print(false, type::message, false, "Stopping [Contract-Task] [" + to_string(task_id) + "]...");
            return;
        }
    }
}

void Action::stop_sniper_task(const Napi::CallbackInfo& info) {
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Sniper_Task::stop_pool.size(); x++) {
        if (Sniper_Task::stop_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Sniper-Task] [" + to_string(task_id) + "] cannot be stopped as it is already not active.");
            return;
        }
    }
    for (int x = 0; x < Sniper_Task::delete_pool.size(); x++) {
        if (Sniper_Task::delete_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Sniper-Task] [" + to_string(task_id) + "] cannot be stopped as it is being deleted.");
            return;
        }
    }
    for (int x = 0; x < Sniper_Task::stop_pool.size(); x++) {
        if (Sniper_Task::stop_pool[x] == 0) {
            Sniper_Task::stop_pool[x] = task_id;
            Instance_Summary::sniper_tasks_active--;
            Environment::main_update_instance_summary(false);
            Chronicle::print(false, type::message, false, "Stopping [Sniper-Task] [" + to_string(task_id) + "]...");
            return;
        }
    }
}

Napi::String Action::view_contract_task(const Napi::CallbackInfo& info) {
    int task_id = info[0].ToNumber().Int32Value();
    ptree view_contract_task_json;
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            view_contract_task_json.put("Task_ID", Module::contract_tasks_[x]->task_id);
            bool active = true;
            for (int y = 0; y < Contract_Task::stop_pool.size(); y++) {
                if (Contract_Task::stop_pool[y] == Module::contract_tasks_[x]->task_id) {
                    active = false;
                    break;
                }
            }
            if (active) {
                view_contract_task_json.put("Active", true);
            } else {
                view_contract_task_json.put("Active", false);
            }
            view_contract_task_json.put("Blockchain", Module::contract_tasks_[x]->blockchain);
            view_contract_task_json.put("Wallet_Group", Module::contract_tasks_[x]->wallet_group.name);
            if (Module::contract_tasks_[x]->blockchain == "Ethereum") {
                view_contract_task_json.put("Ethereum.Contract_Address", Module::contract_tasks_[x]->ethereum_contract_address);
                view_contract_task_json.put("Ethereum.Function", Module::contract_tasks_[x]->ethereum_function);
                view_contract_task_json.put_child("Ethereum.Call_Data", Module::contract_tasks_[x]->ethereum_call_data);
                view_contract_task_json.put("Ethereum.Value", Module::contract_tasks_[x]->ethereum_value);
                view_contract_task_json.put("Ethereum.Gas_Price", Module::contract_tasks_[x]->ethereum_gas_price);
                view_contract_task_json.put("Ethereum.Gas_Limit", Module::contract_tasks_[x]->ethereum_gas_limit);
                view_contract_task_json.put("Ethereum.Priority_Fee", Module::contract_tasks_[x]->ethereum_priority_fee);
            } else if (Module::contract_tasks_[x]->blockchain == "Solana") {
                view_contract_task_json.put("Solana.Mode", Module::contract_tasks_[x]->solana_mode);
                if (Module::contract_tasks_[x]->solana_mode == "CMV1") {
                    view_contract_task_json.put("Solana.Candy_Machine_V1.Id", Module::contract_tasks_[x]->solana_candy_machine_id);
                    view_contract_task_json.put("Solana.Candy_Machine_V1.Amount", Module::contract_tasks_[x]->solana_candy_machine_amount);
                } else if (Module::contract_tasks_[x]->solana_mode == "CMV2") {
                    view_contract_task_json.put("Solana.Candy_Machine_V2.Id", Module::contract_tasks_[x]->solana_candy_machine_id);
                    view_contract_task_json.put("Solana.Candy_Machine_V2.Amount", Module::contract_tasks_[x]->solana_candy_machine_amount);
                }
            }
            view_contract_task_json.put("Private_Key_Instances", Module::contract_tasks_[x]->wallet_group.private_keys.size());
            break;
        }
    }
    string formatted;
    string json = Utility::format_json(view_contract_task_json);
    for (int x = 0; x < json.length(); x++) {
        if (json[x] == '\\') {
            formatted += "\\\\";
        } else {
            if (json[x] == '\'') {
                formatted += "\\'";
            } else {
                formatted += json[x];
            }
        }
    }
    return Napi::String::New(Environment::context->Global().Env(), formatted);
}

Napi::String Action::view_sniper_task(const Napi::CallbackInfo& info) {
    int task_id = info[0].ToNumber().Int32Value();
    ptree view_sniper_tasks_json;
    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
        if (Module::sniper_tasks_[x]->task_id == task_id) {
            view_sniper_tasks_json.put("Task_ID", Module::sniper_tasks_[x]->task_id);
            bool active = true;
            for (int y = 0; y < Sniper_Task::stop_pool.size(); y++) {
                if (Sniper_Task::stop_pool[y] == Module::sniper_tasks_[x]->task_id) {
                    active = false;
                    break;
                }
            }
            if (active) {
                view_sniper_tasks_json.put("Active", true);
            } else {
                view_sniper_tasks_json.put("Active", false);
            }
            view_sniper_tasks_json.put("Blockchain", Module::sniper_tasks_[x]->blockchain);
            view_sniper_tasks_json.put("Marketplace", Module::sniper_tasks_[x]->marketplace);
            view_sniper_tasks_json.put("Marketplace_API_Key", Module::sniper_tasks_[x]->marketplace_api_key);
            view_sniper_tasks_json.put("Collection", Module::sniper_tasks_[x]->collection);
            if (Module::sniper_tasks_[x]->blockchain == "Ethereum") {
                view_sniper_tasks_json.put("Ethereum.Use_Flashbots", Module::sniper_tasks_[x]->use_flashbots);
                view_sniper_tasks_json.put("Ethereum.ERC_20_Token_Address", Module::sniper_tasks_[x]->erc_20_token_address);
                view_sniper_tasks_json.put("Ethereum.Gas_Price", Module::sniper_tasks_[x]->ethereum_gas_price);
                view_sniper_tasks_json.put("Ethereum.Gas_Limit", Module::sniper_tasks_[x]->ethereum_gas_limit);
                view_sniper_tasks_json.put("Ethereum.Priority_Fee", Module::sniper_tasks_[x]->ethereum_priority_fee);
                view_sniper_tasks_json.put("Ethereum.Nonce", Module::sniper_tasks_[x]->ethereum_nonce);
            }
            view_sniper_tasks_json.put("Wallet_Private_Key", Module::sniper_tasks_[x]->wallet_private_key);
            view_sniper_tasks_json.put("Minimum_Value", Module::sniper_tasks_[x]->minimum_value);
            view_sniper_tasks_json.put("Maximum_Value", Module::sniper_tasks_[x]->maximum_value);
            view_sniper_tasks_json.put("Delay", Module::sniper_tasks_[x]->delay);
            break;
        }
    }
    string formatted;
    string json = Utility::format_json(view_sniper_tasks_json);
    for (int x = 0; x < json.length(); x++) {
        if (json[x] == '\\') {
            formatted += "\\\\";
        } else {
            if (json[x] == '\'') {
                formatted += "\\'";
            } else {
                formatted += json[x];
            }
        }
    }
    return Napi::String::New(Environment::context->Global().Env(), formatted);
}

void Action::delete_contract_task(const Napi::CallbackInfo& info) {
    bool task_exists = false;
    bool active = true;
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            task_exists = true;
            if (Module::contract_tasks_[x]->is_polling) {
                Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be deleted as it is polling.");
                return;
            }
        }
    }
    if (!task_exists) {
        Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be deleted as it does not exist.");
        return;
    }
    for (int x = 0; x < Contract_Task::delete_pool.size(); x++) {
        if (Contract_Task::delete_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Contract-Task] [" + to_string(task_id) + "] cannot be deleted as it does not exist.");
            return;
        }
    }
    for (int x = 0; x < Contract_Task::stop_pool.size(); x++) {
        if (Contract_Task::stop_pool[x] == task_id) { active = false;  Contract_Task::stop_pool[x] = 0; }
    }
    for (int x = 0; x < Contract_Task::delete_pool.size(); x++) {
        if (Contract_Task::delete_pool[x] == 0) {
            Contract_Task::delete_pool[x] = task_id;
            if (active) { Instance_Summary::contract_tasks_active--; }
            Instance_Summary::total_contract_tasks--;
            break;
        }
    }
    Environment::main_update_instance_summary(false);
    Chronicle::print(false, type::message, false, "Deleting [Contract-Task] [" + to_string(task_id) + "]...");
}

void Action::delete_sniper_task(const Napi::CallbackInfo& info) {
    bool task_exists = false;
    bool active = true;
    int task_id = info[0].ToNumber().Int32Value();
    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
        if (Module::sniper_tasks_[x]->task_id == task_id) {
            task_exists = true;
        }
    }
    if (!task_exists) {
        Chronicle::print(false, type::warning, false, "[Sniper-Task] [" + to_string(task_id) + "] cannot be deleted as it does not exist.");
        return;
    }
    for (int x = 0; x < Sniper_Task::delete_pool.size(); x++) {
        if (Sniper_Task::delete_pool[x] == task_id) {
            Chronicle::print(false, type::warning, false, "[Sniper-Task] [" + to_string(task_id) + "] cannot be deleted as it does not exist.");
            return;
        }
    }
    for (int x = 0; x < Sniper_Task::stop_pool.size(); x++) {
        if (Sniper_Task::stop_pool[x] == task_id) { active = false;  Sniper_Task::stop_pool[x] = 0; }
    }
    for (int x = 0; x < Sniper_Task::delete_pool.size(); x++) {
        if (Sniper_Task::delete_pool[x] == 0) {
            Sniper_Task::delete_pool[x] = task_id;
            if (active) { Instance_Summary::sniper_tasks_active--; }
            Instance_Summary::total_sniper_tasks--;
            break;
        }
    }
    Environment::main_update_instance_summary(false);
    Chronicle::print(false, type::message, false, "Deleting [Sniper-Task] [" + to_string(task_id) + "]...");
}

void Action::remove_contract_task(int task_id) {
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        if (Module::contract_tasks_[x]->task_id == task_id) {
            Module::contract_tasks_[x].reset();
            Module::contract_tasks_.erase(Module::contract_tasks_.begin() + x);
        }
    }
    ptree temp_contract_tasks_container;
    for (int x = 0; x < Module::contract_tasks_.size(); x++) {
        ptree temp_contract_task;
        temp_contract_task.put("Blockchain", Module::contract_tasks_[x]->blockchain);
        temp_contract_task.put("Wallet_Group", Module::contract_tasks_[x]->wallet_group.name);
        if (Module::contract_tasks_[x]->blockchain == "Ethereum") {
            temp_contract_task.put("Ethereum.Contract_Address", Module::contract_tasks_[x]->ethereum_contract_address);
            temp_contract_task.put("Ethereum.Function", Module::contract_tasks_[x]->ethereum_function);
            temp_contract_task.put_child("Ethereum.Call_Data", Module::contract_tasks_[x]->ethereum_call_data);
            temp_contract_task.put("Ethereum.Value", Module::contract_tasks_[x]->ethereum_value);
            temp_contract_task.put("Ethereum.Gas_Price", Module::contract_tasks_[x]->ethereum_gas_price);
            temp_contract_task.put("Ethereum.Gas_Limit", Module::contract_tasks_[x]->ethereum_gas_limit);
            temp_contract_task.put("Ethereum.Priority_Fee", Module::contract_tasks_[x]->ethereum_priority_fee);
        } else if (Module::contract_tasks_[x]->blockchain == "Solana") {
            temp_contract_task.put("Solana.Mode", Module::contract_tasks_[x]->solana_mode);
            if (Module::contract_tasks_[x]->solana_mode == "CMV1") {
                temp_contract_task.put("Solana.Candy_Machine_V1.Id", Module::contract_tasks_[x]->solana_candy_machine_id);
                temp_contract_task.put("Solana.Candy_Machine_V1.Amount", Module::contract_tasks_[x]->solana_candy_machine_amount);
            } else if (Module::contract_tasks_[x]->solana_mode == "CMV2") {
                temp_contract_task.put("Solana.Candy_Machine_V2.Id", Module::contract_tasks_[x]->solana_candy_machine_id);
                temp_contract_task.put("Solana.Candy_Machine_V2.Amount", Module::contract_tasks_[x]->solana_candy_machine_amount);
            }
        }
        temp_contract_task.put("Time_Started", Module::contract_tasks_[x]->time_started);
        temp_contract_tasks_container.push_back(ptree::value_type("", temp_contract_task));
    }
    ptree save_contract_tasks;
    save_contract_tasks.put_child("Contract_Tasks", temp_contract_tasks_container);
    write_json(Module::persistent_path + "UserData/ContractTasks/contract_tasks.json", save_contract_tasks);
    Environment::main_update_contract_tasks(true);
}

void Action::remove_sniper_task(int task_id) {
    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
        if (Module::sniper_tasks_[x]->task_id == task_id) {
            Module::sniper_tasks_[x].reset();
            Module::sniper_tasks_.erase(Module::sniper_tasks_.begin() + x);
        }
    }
    ptree temp_sniper_tasks_container;
    for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
        ptree temp_sniper_tasks;
        temp_sniper_tasks.put("Blockchain", Module::sniper_tasks_[x]->blockchain);
        temp_sniper_tasks.put("Marketplace", Module::sniper_tasks_[x]->marketplace);
        temp_sniper_tasks.put("Marketplace_API_Key", Module::sniper_tasks_[x]->marketplace_api_key);
        temp_sniper_tasks.put("Collection", Module::sniper_tasks_[x]->collection);
        if (Module::sniper_tasks_[x]->blockchain == "Ethereum") {
            temp_sniper_tasks.put("Ethereum.Use_Flashbots", Module::sniper_tasks_[x]->use_flashbots);
            temp_sniper_tasks.put("Ethereum.ERC_20_Token_Address", Module::sniper_tasks_[x]->erc_20_token_address);
            temp_sniper_tasks.put("Ethereum.Gas_Price", Module::sniper_tasks_[x]->ethereum_gas_price);
            temp_sniper_tasks.put("Ethereum.Gas_Limit", Module::sniper_tasks_[x]->ethereum_gas_limit);
            temp_sniper_tasks.put("Ethereum.Priority_Fee", Module::sniper_tasks_[x]->ethereum_priority_fee);
            temp_sniper_tasks.put("Ethereum.Nonce", Module::sniper_tasks_[x]->ethereum_nonce);
        }
        temp_sniper_tasks.put("Wallet_Private_Key", Cryptography::aes_encrypt(Module::sniper_tasks_[x]->wallet_private_key));
        temp_sniper_tasks.put("Minimum_Value", Module::sniper_tasks_[x]->minimum_value);
        temp_sniper_tasks.put("Maximum_Value", Module::sniper_tasks_[x]->maximum_value);
        temp_sniper_tasks.put("Delay", Module::sniper_tasks_[x]->delay);
        temp_sniper_tasks_container.push_back(ptree::value_type("", temp_sniper_tasks));
    }
    ptree save_sniper_tasks;
    save_sniper_tasks.put_child("Sniper_Tasks", temp_sniper_tasks_container);
    write_json(Module::persistent_path + "UserData/SniperTasks/sniper_tasks.json", save_sniper_tasks);
    Environment::main_update_sniper_tasks(true);
}

void Action::generate_wallet(const Napi::CallbackInfo& info) {
    stringstream json;
    ptree base;
    ptree encrypted;
    ptree response;
    string api;
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, base);
        base.get<string>("Blockchain");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to generate wallet.");
        return;
    }
    if (base.get<string>("Blockchain") != "Ethereum" && base.get<string>("Blockchain") != "Solana") {
        Chronicle::print(false, type::error, false, "Blockchain [" + base.get<string>("Blockchain") + "] is not supported.");
        return;
    } else if (base.get<string>("Blockchain") == "Ethereum") {
        api = "ethers";
        base.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
        base.put("ethereum.chain_id", Settings::ethereum_chain_id);
    } else if (base.get<string>("Blockchain") == "Solana") {
        api = "solana";
        if (Settings::solana_rpc_urls.size() > 0) {
            base.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
        }
    }
    encrypted.put("session", Auth::current_session);
    base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
    response = Http_Request::send(verb::post, Action::host, Action::port, ("/api/v4/user/" + api + "/generate_wallet").c_str(), Utility::format_json(base).c_str(), "text/plain");
    if (response.size() == 0) {
        Chronicle::print(false, type::error, false, "Failed to send request, check your internet/client's connection and try again.");
        return;
    } else {
        if (!response.get<bool>("success")) {
            Chronicle::print(false, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
        } else if (response.get<bool>("success")) {
            Chronicle::print(false, type::message, false, "[" + base.get<string>("Blockchain") + "-Public-Address-" + response.get<string>("public_address") + "]");
            Chronicle::print(false, type::message, false, "[" + base.get<string>("Blockchain") + "-Private-Key-" + response.get<string>("private_key") + "]");
        }
    }
}

void Action::create_wallet_group(const Napi::CallbackInfo& info) {
    stringstream json;
    ptree container;
    ptree base;
    ptree encrypted;
    ptree response;
    string api;
    json << info[0].ToString().Utf8Value();
    try {
        read_json(json, container);
        container.get<string>("Name");
        container.get<string>("Blockchain");
        container.get_child("Private_Keys");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to create [Wallet-Group].");
        return;
    }
    if (container.get<string>("Name").size() < 1) {
        Chronicle::print(false, type::error, false, "[Wallet-Group] name must be more than 0 characters.");
        return;
    } else if (container.get<string>("Name").size() > 17) {
        Chronicle::print(false, type::error, false, "[Wallet-Group] name must be less than 17 characters.");
        return;
    }
    Wallet_Group temp_wallet_group;
    for (int x = 0; x < Module::wallet_groups.size(); x++) {
        if (Module::wallet_groups[x].name == container.get<string>("Name")) {
            Chronicle::print(false, type::error, false, "A [Wallet-Group] using this name already exists.");
            return;
        }
    }
    temp_wallet_group.name = container.get<string>("Name");
    if (container.get<string>("Blockchain") != "Ethereum" && container.get<string>("Blockchain") != "Solana") {
        Chronicle::print(false, type::error, false, "Blockchain [" + container.get<string>("Blockchain") + "] is not supported.");
        return;
    } else if (container.get<string>("Blockchain") == "Ethereum") {
        api = "ethers";
        base.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
        base.put("ethereum.chain_id", Settings::ethereum_chain_id);
    } else if (container.get<string>("Blockchain") == "Solana") {
        api = "solana";
        if (Settings::solana_rpc_urls.size() > 0) {
            base.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
        }
    }
    temp_wallet_group.blockchain = container.get<string>("Blockchain");
    BOOST_FOREACH(ptree::value_type & instance, container.get_child("Private_Keys")) {
        assert(instance.first.empty());
        temp_wallet_group.private_keys.push_back(instance.second.data());
    }
    ptree temp_private_keys;
    for (int x = 0; x < temp_wallet_group.private_keys.size(); x++) {
        ptree node;
        node.put("", temp_wallet_group.private_keys[x]);
        temp_private_keys.push_back(std::make_pair("", node));
    }
    encrypted.put("session", Auth::current_session);
    base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
    base.put_child("private_keys", temp_private_keys);
    response = Http_Request::send(verb::post, Action::host, Action::port, ("/api/v4/user/" + api + "/wallet_group_balance").c_str(), Utility::format_json(base).c_str(), "text/plain");
    if (response.size() == 0) {
        Chronicle::print(false, type::error, false, "Failed to send request, check your internet/client's connection and try again.");
        return;
    } else {
        if (!response.get<bool>("success")) {
            Chronicle::print(false, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
        } else if (response.get<bool>("success")) {
            Wallet_Group::metadata info;
            info.name = container.get<string>("Name");
            info.value = response.get<float>("value");
            info.acronym = response.get<string>("acronym");
            if (info.acronym == "ETH") {
                info.market_value = info.value * Instance_Summary::ethereum_value;
            } else if (info.acronym == "SOL") {
                info.market_value = info.value * Instance_Summary::solana_value;
            }
            Wallet_Group::info.push_back(info);
            ptree temp_metadata_container;
            for (int x = 0; x < Wallet_Group::info.size(); x++) {
                ptree temp_metadata;
                temp_metadata.put("Name", Wallet_Group::info[x].name);
                temp_metadata.put("Value", Wallet_Group::info[x].value);
                temp_metadata.put("Market_Value", Wallet_Group::info[x].market_value);
                temp_metadata.put("Acronym", Wallet_Group::info[x].acronym);
                temp_metadata_container.push_back(ptree::value_type("", temp_metadata));
            }
            Module::wallet_groups.push_back(temp_wallet_group);
            ptree temp_wallet_groups_container;
            for (int x = 0; x < Module::wallet_groups.size(); x++) {
                ptree temp_wallet_group;
                ptree temp_private_keys;
                temp_wallet_group.put("Name", Module::wallet_groups[x].name);
                temp_wallet_group.put("Blockchain", Module::wallet_groups[x].blockchain);
                for (int y = 0; y < Module::wallet_groups[x].private_keys.size(); y++) {
                    ptree node;
                    node.put("", Module::wallet_groups[x].private_keys[y]);
                    temp_private_keys.push_back(std::make_pair("", node));
                }
                temp_wallet_group.put_child("Private_Keys", temp_private_keys);
                temp_wallet_groups_container.push_back(ptree::value_type("", temp_wallet_group));
            }
            ptree save_wallet_groups;
            save_wallet_groups.put_child("Wallet_Groups", temp_wallet_groups_container);
            save_wallet_groups.put_child("Metadata", temp_metadata_container);
            ofstream encrypted_wallet_groups;
            encrypted_wallet_groups.open(Module::persistent_path + "UserData/WalletGroups/wallet_groups.json");
            encrypted_wallet_groups << Cryptography::aes_encrypt(Utility::format_json(save_wallet_groups));
            encrypted_wallet_groups.close();
            Environment::main_update_wallet_groups();
            Chronicle::print(false, type::message, false, "Successfully created and acquired information for [Wallet-Group] [" + temp_wallet_group.name + "].");
        }
    }
}

Napi::String Action::view_wallet_group(const Napi::CallbackInfo& info) {
    string wallet_group_name = info[0].ToString().Utf8Value();
    ptree view_wallet_group_private_keys_json;
    ptree temp_private_keys;
    for (int x = 0; x < Module::wallet_groups.size(); x++) {
        if (Module::wallet_groups[x].name == wallet_group_name) {
            for (int y = 0; y < Module::wallet_groups[x].private_keys.size(); y++) {
                ptree node;
                node.put("", Module::wallet_groups[x].private_keys[y]);
                temp_private_keys.push_back(std::make_pair("", node));
            }
            break;
        }
    }
    view_wallet_group_private_keys_json.put_child("Private_Keys", temp_private_keys);
    return Napi::String::New(Environment::context->Global().Env(), Utility::format_json(view_wallet_group_private_keys_json));
}

void Action::edit_wallet_group(const Napi::CallbackInfo& info) {
    string wallet_group_name = info[0].ToString().Utf8Value();
    stringstream json;
    ptree container;
    ptree base;
    ptree encrypted;
    ptree response;
    string api;
    json << info[1].ToString().Utf8Value();
    try {
        read_json(json, container);
        container.get<string>("Private_Keys");
    } catch (exception const& e) {
        Chronicle::print(false, type::error, false, "Could not read the json submitted to edit [Wallet-Group] [" + wallet_group_name + "].");
        return;
    }
    for (int x = 0; x < Module::wallet_groups.size(); x++) {
        if (Module::wallet_groups[x].name == wallet_group_name) {
            if (Module::wallet_groups[x].blockchain == "Ethereum") {
                api = "ethers";
                base.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
                base.put("ethereum.chain_id", Settings::ethereum_chain_id);
            } else if (Module::wallet_groups[x].blockchain == "Solana") {
                api = "solana";
                if (Settings::solana_rpc_urls.size() > 0) {
                    base.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
                }
            }
        }
    }
    vector<string> private_keys;
    BOOST_FOREACH(ptree::value_type & instance, container.get_child("Private_Keys")) {
        assert(instance.first.empty());
        private_keys.push_back(instance.second.data());
    }
    ptree temp_private_keys;
    for (int x = 0; x < private_keys.size(); x++) {
        ptree node;
        node.put("", private_keys[x]);
        temp_private_keys.push_back(std::make_pair("", node));
    }
    encrypted.put("session", Auth::current_session);
    base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
    base.put_child("private_keys", temp_private_keys);
    response = Http_Request::send(verb::post, Action::host, Action::port, ("/api/v4/user/" + api + "/wallet_group_balance").c_str(), Utility::format_json(base).c_str(), "text/plain");
    if (response.size() == 0) {
        Chronicle::print(false, type::error, false, "Failed to send request, check your internet/client's connection and try again.");
        return;
    } else {
        if (!response.get<bool>("success")) {
            Chronicle::print(false, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
        } else if (response.get<bool>("success")) {
            for (int x = 0; x < Module::wallet_groups.size(); x++) {
                if (Module::wallet_groups[x].name == wallet_group_name) {
                    Module::wallet_groups[x].private_keys = private_keys;
                }
            }
            ptree temp_wallet_groups_container;
            for (int x = 0; x < Module::wallet_groups.size(); x++) {
                ptree temp_wallet_group;
                ptree temp_private_keys;
                temp_wallet_group.put("Name", Module::wallet_groups[x].name);
                temp_wallet_group.put("Blockchain", Module::wallet_groups[x].blockchain);
                for (int y = 0; y < Module::wallet_groups[x].private_keys.size(); y++) {
                    ptree node;
                    node.put("", Module::wallet_groups[x].private_keys[y]);
                    temp_private_keys.push_back(std::make_pair("", node));
                }
                temp_wallet_group.put_child("Private_Keys", temp_private_keys);
                temp_wallet_groups_container.push_back(ptree::value_type("", temp_wallet_group));
            }
            Wallet_Group::metadata info;
            info.name = wallet_group_name;
            info.value = response.get<float>("value");
            info.acronym = response.get<string>("acronym");
            if (info.acronym == "ETH") {
                info.market_value = info.value * Instance_Summary::ethereum_value;
            } else if (info.acronym == "SOL") {
                info.market_value = info.value * Instance_Summary::solana_value;
            }
            for (int x = 0; x < Wallet_Group::info.size(); x++) {
                if (wallet_group_name == Wallet_Group::info[x].name) {
                    Wallet_Group::info[x] = info;
                }
            }
            ptree temp_metadata_container;
            for (int x = 0; x < Wallet_Group::info.size(); x++) {
                ptree temp_metadata;
                temp_metadata.put("Name", Wallet_Group::info[x].name);
                temp_metadata.put("Value", Wallet_Group::info[x].value);
                temp_metadata.put("Market_Value", Wallet_Group::info[x].market_value);
                temp_metadata.put("Acronym", Wallet_Group::info[x].acronym);
                temp_metadata_container.push_back(ptree::value_type("", temp_metadata));
            }
            ptree save_wallet_groups;
            save_wallet_groups.put_child("Wallet_Groups", temp_wallet_groups_container);
            save_wallet_groups.put_child("Metadata", temp_metadata_container);
            ofstream encrypted_wallet_groups;
            encrypted_wallet_groups.open(Module::persistent_path + "UserData/WalletGroups/wallet_groups.json");
            encrypted_wallet_groups << Cryptography::aes_encrypt(Utility::format_json(save_wallet_groups));
            encrypted_wallet_groups.close();
            Environment::main_update_wallet_groups();
            Chronicle::print(false, type::message, false, "Successfully edited information for [Wallet-Group] [" + wallet_group_name + "].");
        }
    }
}

void Action::delete_wallet_group(const Napi::CallbackInfo& info) {
    string wallet_group_name = info[0].ToString().Utf8Value();
    for (int x = 0; x < Module::wallet_groups.size(); x++) {
        if (Module::wallet_groups[x].name == wallet_group_name) { Module::wallet_groups.erase(Module::wallet_groups.begin() + x); }
        if (Wallet_Group::info[x].name == wallet_group_name) { Wallet_Group::info.erase(Wallet_Group::info.begin() + x); }
    }
    ptree temp_wallet_groups_container;
    for (int x = 0; x < Module::wallet_groups.size(); x++) {
        ptree temp_wallet_group;
        ptree temp_private_keys;
        temp_wallet_group.put("Name", Module::wallet_groups[x].name);
        temp_wallet_group.put("Blockchain", Module::wallet_groups[x].blockchain);
        for (int y = 0; y < Module::wallet_groups[x].private_keys.size(); y++) {
            ptree node;
            node.put("", Module::wallet_groups[x].private_keys[y]);
            temp_private_keys.push_back(std::make_pair("", node));
        }
        temp_wallet_group.put_child("Private_Keys", temp_private_keys);
        temp_wallet_groups_container.push_back(ptree::value_type("", temp_wallet_group));
    }
    ptree temp_metadata_container;
    for (int x = 0; x < Wallet_Group::info.size(); x++) {
        ptree temp_metadata;
        temp_metadata.put("Name", Wallet_Group::info[x].name);
        temp_metadata.put("Value", Wallet_Group::info[x].value);
        temp_metadata.put("Market_Value", Wallet_Group::info[x].market_value);
        temp_metadata.put("Acronym", Wallet_Group::info[x].acronym);
        temp_metadata_container.push_back(ptree::value_type("", temp_metadata));
    }
    ptree save_wallet_groups;
    save_wallet_groups.put_child("Wallet_Groups", temp_wallet_groups_container);
    save_wallet_groups.put_child("Metadata", temp_metadata_container);
    ofstream encrypted_wallet_groups;
    encrypted_wallet_groups.open(Module::persistent_path + "UserData/WalletGroups/wallet_groups.json");
    encrypted_wallet_groups << Cryptography::aes_encrypt(Utility::format_json(save_wallet_groups));
    encrypted_wallet_groups.close();
    Environment::main_update_wallet_groups();
    Chronicle::print(false, type::message, false, "Successfully deleted [Wallet-Group] [" + wallet_group_name + "].");
    return;
}

void Action::refresh_balances(const Napi::CallbackInfo& info) {
    ptree encrypted;
    ptree base;
    ptree response;
    string api;
    vector<Wallet_Group::metadata> new_info;
    encrypted.put("session", Auth::current_session);
    base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
    for (int x = 0; x < Module::wallet_groups.size(); x++) {
        ptree temp_private_keys;
        for (int y = 0; y < Module::wallet_groups[x].private_keys.size(); y++) {
            ptree node;
            node.put("", Module::wallet_groups[x].private_keys[y]);
            temp_private_keys.push_back(std::make_pair("", node));
        }
        base.put_child("private_keys", temp_private_keys);
        if (Module::wallet_groups[x].blockchain == "Ethereum") {
            api = "ethers";
            base.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
            base.put("ethereum.chain_id", Settings::ethereum_chain_id);
        } else if (Module::wallet_groups[x].blockchain == "Solana") {
            api = "solana";
            if (Settings::solana_rpc_urls.size() > 0) {
                base.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
            }
        }
        response = Http_Request::send(verb::post, Action::host, Action::port, ("/api/v4/user/" + api + "/wallet_group_balance").c_str(), Utility::format_json(base).c_str(), "text/plain");
        if (response.size() == 0) {
            Chronicle::print(false, type::error, false, "Failed to update one or more [Wallet-Group](s), check your internet/client's connection and try again.");
            return;
        } else {
            if (!response.get<bool>("success")) {
                Chronicle::print(false, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
                return;
            } else if (response.get<bool>("success")) {
                Wallet_Group::metadata info;
                info.name = Module::wallet_groups[x].name;
                info.value = response.get<float>("value");
                info.acronym = response.get<string>("acronym");
                if (info.acronym == "ETH") {
                    info.market_value = info.value * Instance_Summary::ethereum_value;
                } else if (info.acronym == "SOL") {
                    info.market_value = info.value * Instance_Summary::solana_value;
                }
                new_info.push_back(info);
            }
        }
    }
    Wallet_Group::info = new_info;
    ptree temp_metadata_container;
    for (int x = 0; x < Wallet_Group::info.size(); x++) {
        ptree temp_metadata;
        temp_metadata.put("Name", Wallet_Group::info[x].name);
        temp_metadata.put("Value", Wallet_Group::info[x].value);
        temp_metadata.put("Market_Value", Wallet_Group::info[x].market_value);
        temp_metadata.put("Acronym", Wallet_Group::info[x].acronym);
        temp_metadata_container.push_back(ptree::value_type("", temp_metadata));
    }
    Module::wallet_groups_container.put_child("Metadata", temp_metadata_container);
    ofstream encrypted_wallet_groups;
    encrypted_wallet_groups.open(Module::persistent_path + "UserData/WalletGroups/wallet_groups.json");
    encrypted_wallet_groups << Cryptography::aes_encrypt(Utility::format_json(Module::wallet_groups_container));
    encrypted_wallet_groups.close();
    Environment::main_update_wallet_groups();
    Chronicle::print(false, type::message, false, "Successfully refreshed balances.");
}

void Action::add_transaction(bool threaded, string blockchain, string txn_hash) {
    if (threaded) {
        if (blockchain == "Ethereum") {
            Environment::client_poll_ethereum_transaction(txn_hash, Settings::ethereum_https_provider_api_url, Settings::ethereum_chain_id, Settings::discord_webhook);
        }
    }
    Transaction new_txn;
    new_txn.blockchain = blockchain;
    new_txn.transaction_hash = txn_hash;
    new_txn.date_initiated = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    Module::todays_transactions.push_back(new_txn);
    Environment::main_update_transaction_history(threaded, true);
    for (int x = 0; x < Module::todays_transactions.size(); x++) {
        if (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - Module::todays_transactions[x].date_initiated > 86400000) {
            Module::todays_transactions.erase(Module::todays_transactions.begin() + x);
        }
    }
    Environment::main_update_todays_transactions(threaded);
    ptree new_txn_json;
    new_txn_json.put("Blockchain", new_txn.blockchain);
    new_txn_json.put("Transaction_Hash", new_txn.transaction_hash);
    new_txn_json.put("Date_Initiated", new_txn.date_initiated);
    if (!Module::transaction_history.get_child_optional("Transaction_Library").has_value()) {
        ptree transaction_array;
        transaction_array.push_back(ptree::value_type("", new_txn_json));
        Module::transaction_history.put_child("Transaction_Library", transaction_array);
    } else {
        Module::transaction_history.get_child("Transaction_Library").push_back(ptree::value_type("", new_txn_json));
    }
    write_json(Module::persistent_path + "UserData/Transactions/transaction_library.json", Module::transaction_history);
    Analytics::total_transactions++;
    Instance_Summary::txns_attempted++;
    Environment::main_update_analytics(threaded);
    Environment::main_update_instance_summary(threaded);
    Chronicle::print(threaded, type::message, false, "Attempted [Transaction] [" + new_txn.transaction_hash + "].");
}

void Action::open_href(const Napi::CallbackInfo& info) {
    stringstream json;
    ptree container;
    json << info[0].ToString().Utf8Value();
    read_json(json, container);
    if (container.get_optional<string>("wallet").has_value()) {
        system(("start https://etherscan.io/address/" + container.get<string>("wallet")).c_str());
    } else if (container.get_optional<string>("email").has_value()) {
        system(("start https://moondweller.hyper.co"));
    } else if (container.get_optional<string>("txn").has_value()) {
        if (container.get<string>("blockchain") == "Ethereum") {
            system(("start https://etherscan.io/tx/" + container.get<string>("txn")).c_str());
        } else if (container.get<string>("blockchain") == "Solana") {
            system(("start https://explorer.solana.com/tx/" + container.get<string>("txn")).c_str());
        }
    } else if (container.get_optional<string>("contract").has_value()) {
        if (container.get<string>("blockchain") == "Ethereum") {
            system(("start https://etherscan.io/address/" + container.get<string>("contract")).c_str());
        } else if (container.get<string>("blockchain") == "Solana") {
            system(("start https://explorer.solana.com/address/" + container.get<string>("contract")).c_str());
        }
    } else if (container.get_optional<string>("slug").has_value()) {
        if (container.get<string>("marketplace") == "opensea") {
            system(("start https://opensea.io/collection/" + container.get<string>("slug")).c_str());
        } else if (container.get<string>("marketplace") == "magiceden") {
            system(("start https://magiceden.io/marketplace/" + container.get<string>("slug")).c_str());
        }
    }
}