#include <napi.h>
#include <boost/thread.hpp>
#include "Environment.h"
#include "Session.h"
#include "Action.h"
#include "Auth.h"

string Module::persistent_path;
string Module::client_version = "1.0.1";
atomic<bool> Module::up_to_date = true;
vector<unique_ptr<Contract_Task>> Module::contract_tasks_;
ptree Module::contract_tasks_container;
vector<unique_ptr<Sniper_Task>> Module::sniper_tasks_;
ptree Module::sniper_tasks_container;
vector<Wallet_Group> Module::wallet_groups;
ptree Module::wallet_groups_container;
vector<Transaction> Module::todays_transactions;
ptree Module::transaction_history;

void main_function(const Napi::CallbackInfo& info) {
    Module::persistent_path = "C:/Users/" + string(getenv("username")) + "/AppData/Roaming/MoonDweller/";
    boost::filesystem::create_directory(Module::persistent_path + "Chronicles");
    Auth::load_hyper_auth_cert();
    Chronicle::session = Chronicle::get_session();
    boost::thread{ &Session::new_session };
}

Napi::Object Init(Napi::Env environment, Napi::Object exports) {
    Environment::context.reset(new Napi::Env(environment));
    exports.Set(Napi::String::New(environment, "bridge_functions"), Napi::Function::New(environment, Environment::bridge_functions));
    exports.Set(Napi::String::New(environment, "main_function"), Napi::Function::New(environment, main_function));
    exports.Set(Napi::String::New(environment, "login"), Napi::Function::New(environment, Action::login));
    exports.Set(Napi::String::New(environment, "key_derivation"), Napi::Function::New(environment, Action::key_derivation));
    exports.Set(Napi::String::New(environment, "decrypt"), Napi::Function::New(environment, Action::decrypt));
    exports.Set(Napi::String::New(environment, "view_settings"), Napi::Function::New(environment, Action::view_settings));
    exports.Set(Napi::String::New(environment, "set_settings"), Napi::Function::New(environment, Action::set_settings));
    exports.Set(Napi::String::New(environment, "total_value_spent"), Napi::Function::New(environment, Environment::main_update_total_value_spent));
    exports.Set(Napi::String::New(environment, "create_contract_task"), Napi::Function::New(environment, Action::create_contract_task));
    exports.Set(Napi::String::New(environment, "start_polling_task"), Napi::Function::New(environment, Action::start_polling_task));
    exports.Set(Napi::String::New(environment, "start_transaction_polling_task"), Napi::Function::New(environment, Action::start_transaction_polling_task));
    exports.Set(Napi::String::New(environment, "unlock_polling_task"), Napi::Function::New(environment, Action::unlock_polling_task));
    exports.Set(Napi::String::New(environment, "create_sniper_task"), Napi::Function::New(environment, Action::create_sniper_task));
    exports.Set(Napi::String::New(environment, "start_contract_task"), Napi::Function::New(environment, Action::start_contract_task));
    exports.Set(Napi::String::New(environment, "start_sniper_task"), Napi::Function::New(environment, Action::start_sniper_task));
    exports.Set(Napi::String::New(environment, "stop_contract_task"), Napi::Function::New(environment, Action::stop_contract_task));
    exports.Set(Napi::String::New(environment, "stop_sniper_task"), Napi::Function::New(environment, Action::stop_sniper_task));
    exports.Set(Napi::String::New(environment, "view_contract_task"), Napi::Function::New(environment, Action::view_contract_task));
    exports.Set(Napi::String::New(environment, "view_sniper_task"), Napi::Function::New(environment, Action::view_sniper_task));
    exports.Set(Napi::String::New(environment, "delete_contract_task"), Napi::Function::New(environment, Action::delete_contract_task));
    exports.Set(Napi::String::New(environment, "delete_sniper_task"), Napi::Function::New(environment, Action::delete_sniper_task));
    exports.Set(Napi::String::New(environment, "generate_wallet"), Napi::Function::New(environment, Action::generate_wallet));
    exports.Set(Napi::String::New(environment, "create_wallet_group"), Napi::Function::New(environment, Action::create_wallet_group));
    exports.Set(Napi::String::New(environment, "view_wallet_group"), Napi::Function::New(environment, Action::view_wallet_group));
    exports.Set(Napi::String::New(environment, "edit_wallet_group"), Napi::Function::New(environment, Action::edit_wallet_group));
    exports.Set(Napi::String::New(environment, "delete_wallet_group"), Napi::Function::New(environment, Action::delete_wallet_group));
    exports.Set(Napi::String::New(environment, "refresh_balances"), Napi::Function::New(environment, Action::refresh_balances));
    exports.Set(Napi::String::New(environment, "open_href"), Napi::Function::New(environment, Action::open_href));
    return exports;
}

NODE_API_MODULE(Module, Init)