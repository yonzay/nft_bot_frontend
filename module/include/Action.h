#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <string>
#include <napi.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <Windows.h>

using namespace std;
using namespace boost::property_tree;

class Action {
	static const char* host;
	static const char* port;
public:
	static void login(const Napi::CallbackInfo& info);
	static void key_derivation(const Napi::CallbackInfo& info);
	static void decrypt(const Napi::CallbackInfo& info);
	static void load_user_data();
	static Napi::String view_settings(const Napi::CallbackInfo& info);
	static void set_settings(const Napi::CallbackInfo& info);
	static void update_analytics(bool threaded);
	static void create_contract_task(const Napi::CallbackInfo& info);
	static void start_polling_task(const Napi::CallbackInfo& info);
	static void start_transaction_polling_task(const Napi::CallbackInfo& info);
	static void unlock_polling_task(const Napi::CallbackInfo& info);
	static void create_sniper_task(const Napi::CallbackInfo& info);
	static void start_contract_task(const Napi::CallbackInfo& info);
	static void start_sniper_task(const Napi::CallbackInfo& info);
	static void stop_contract_task(const Napi::CallbackInfo& info);
	static void stop_sniper_task(const Napi::CallbackInfo& info);
	static Napi::String view_contract_task(const Napi::CallbackInfo& info);
	static Napi::String view_sniper_task(const Napi::CallbackInfo& info);
	static void delete_contract_task(const Napi::CallbackInfo& info);
	static void delete_sniper_task(const Napi::CallbackInfo& info);
	static void remove_contract_task(int task_id);
	static void remove_sniper_task(int task_id);
	static void generate_wallet(const Napi::CallbackInfo& info);
	static void create_wallet_group(const Napi::CallbackInfo& info);
	static Napi::String view_wallet_group(const Napi::CallbackInfo& info);
	static void edit_wallet_group(const Napi::CallbackInfo& info);
	static void delete_wallet_group(const Napi::CallbackInfo& info);
	static void refresh_balances(const Napi::CallbackInfo& info);
	static void add_transaction(bool threaded, string blockchain, string txn_hash);
	static void open_href(const Napi::CallbackInfo& info);
};