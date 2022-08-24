#pragma once
#include <string>
#include <vector>
#include <napi.h>

using namespace std;

class Environment {
public:
	static unique_ptr<Napi::Env> context;
	static vector<Napi::ThreadSafeFunction> thread_safe_function;
	static vector<Napi::FunctionReference> function_reference;
	static void bridge_functions(const Napi::CallbackInfo& info);
	static void chronicle_insert_log(bool threaded, string type, string input);
	static void title_bar_update_content(string status_indicator_color, string status_info);
	static void main_update_instance_summary(bool threaded);
	static void main_update_subscription_info();
	static void main_update_dashboard_card();
	static void main_update_analytics(bool threaded);
	static void main_update_total_value_spent(const Napi::CallbackInfo& info);
	static void main_update_todays_transactions(bool threaded);
	static void main_update_contract_tasks(bool threaded);
	static void main_update_sniper_tasks(bool threaded);
	static void main_update_wallet_groups();
	static void main_update_transaction_history(bool threaded, bool insert);
	static void client_create_public_variable_poll_head(int task_id, string https_url, int chain_id, string contract_address, string variable_name, int value, int timeout);
	static void client_create_pending_transaction_poll_head(int task_id, string wss_url, int chain_id, string contract_address, string owner_address, string function_name, int timeout);
	static void client_poll_ethereum_transaction(string txn_hash, string https_url, int chain_id, string discord_webhook);
	static void candy_machine_poll(int task_id, string rpc_url, string address, string type);
};