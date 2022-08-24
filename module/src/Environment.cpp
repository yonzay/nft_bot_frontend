#include "Environment.h"
#include "Chronicle.h"
#include "Auth.h"
#include "Module.h"
#include "Instance_Summary.h"
#include "Analytics.h"
#include "Module.h"

unique_ptr<Napi::Env> Environment::context;

vector<Napi::ThreadSafeFunction> Environment::thread_safe_function;
vector<Napi::FunctionReference> Environment::function_reference;

void Environment::bridge_functions(const Napi::CallbackInfo& info) {
	for (int x = 0; x < info.Length(); x++) {
		Environment::thread_safe_function.push_back(Napi::ThreadSafeFunction::New(Environment::context->Global().Env(), info[x].As<Napi::Function>(), "", 0, 1));
		Environment::function_reference.push_back(Napi::Persistent(info[x].As<Napi::Function>()));
	}
	Environment::function_reference[0].Call({});
	Environment::function_reference[1].Call({});
}

void Environment::chronicle_insert_log(bool threaded, string type, string input) {
	if (threaded) {
		Environment::thread_safe_function[8].BlockingCall([type, input](Napi::Env env, Napi::Function function) {
			Napi::Object insert_log_object = Napi::Object::New(Environment::context->Global().Env());
			insert_log_object.Set("user", Chronicle::user);
			insert_log_object.Set("type", type);
			insert_log_object.Set("input", input);
			function.Call({ Napi::String::New(Environment::context->Global().Env(), "chronicle_insert_log"), insert_log_object });
		});
	} else {
		Napi::Object insert_log_object = Napi::Object::New(Environment::context->Global().Env());
		insert_log_object.Set("user", Chronicle::user);
		insert_log_object.Set("type", type);
		insert_log_object.Set("input", input);
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "chronicle_insert_log"), insert_log_object });
	}
}

void Environment::title_bar_update_content(string status_indicator_color, string status_info) {
	Napi::Object title_bar_object = Napi::Object::New(Environment::context->Global().Env());
	title_bar_object.Set("user_info", Auth::discord_username);
	title_bar_object.Set("user_info_email", Auth::email_address);
	title_bar_object.Set("status_indicator_color", status_indicator_color);
	title_bar_object.Set("status_info", status_info);
	title_bar_object.Set("version", Module::client_version);
	Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "title_bar_update_content"), title_bar_object });
}

void Environment::main_update_instance_summary(bool threaded) {
	if (threaded) {
		Environment::thread_safe_function[8].BlockingCall([](Napi::Env env, Napi::Function function) {
			Napi::Object instance_summary_object = Napi::Object::New(Environment::context->Global().Env());
			instance_summary_object.Set("ethereum_spent", Instance_Summary::ethereum_spent.load());
			instance_summary_object.Set("solana_spent", Instance_Summary::solana_spent.load());
			instance_summary_object.Set("current_gas", Instance_Summary::current_gas.load());
			instance_summary_object.Set("txns_attempted", Instance_Summary::txns_attempted.load());
			instance_summary_object.Set("total_contract_tasks", Instance_Summary::total_contract_tasks.load());
			instance_summary_object.Set("contract_tasks_active", Instance_Summary::contract_tasks_active.load());
			instance_summary_object.Set("total_sniper_tasks", Instance_Summary::total_sniper_tasks.load());
			instance_summary_object.Set("sniper_tasks_active", Instance_Summary::sniper_tasks_active.load());
			function.Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_instance_summary"), instance_summary_object });
		});
	} else {
		Napi::Object instance_summary_object = Napi::Object::New(Environment::context->Global().Env());
		instance_summary_object.Set("ethereum_spent", Instance_Summary::ethereum_spent.load());
		instance_summary_object.Set("solana_spent", Instance_Summary::solana_spent.load());
		instance_summary_object.Set("current_gas", Instance_Summary::current_gas.load());
		instance_summary_object.Set("txns_attempted", Instance_Summary::txns_attempted.load());
		instance_summary_object.Set("total_contract_tasks", Instance_Summary::total_contract_tasks.load());
		instance_summary_object.Set("contract_tasks_active", Instance_Summary::contract_tasks_active.load());
		instance_summary_object.Set("total_sniper_tasks", Instance_Summary::total_sniper_tasks.load());
		instance_summary_object.Set("sniper_tasks_active", Instance_Summary::sniper_tasks_active.load());
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_instance_summary"), instance_summary_object });
	}
}

void Environment::main_update_subscription_info() {
	Napi::Object subscription_info_object = Napi::Object::New(Environment::context->Global().Env());
	subscription_info_object.Set("renewal_date", Auth::renewal_date);
	subscription_info_object.Set("last_login", Auth::last_login);
	subscription_info_object.Set("wallet_address", Auth::wallet_address);
	Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_subscription_info"), subscription_info_object });
}

void Environment::main_update_dashboard_card() {
	Napi::Object dashboard_card_object = Napi::Object::New(Environment::context->Global().Env());
	string username;
	for (int x = 0; x < Auth::discord_username.length(); x++) {
		if (Auth::discord_username[x] == '#') {
			username = Auth::discord_username.substr(0, x);
		}
	}
	dashboard_card_object.Set("nft_image_url", Auth::nft_image_url);
	dashboard_card_object.Set("nft_iteration", Auth::nft_iteration.load());
	if (Auth::auto_login) {
		dashboard_card_object.Set("welcome_message", "Welcome back");
	} else {
		dashboard_card_object.Set("welcome_message", "Welcome");
	}
	dashboard_card_object.Set("username", username);
	dashboard_card_object.Set("quote", Auth::quote);
	Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_dashboard_card"), dashboard_card_object });
}

void Environment::main_update_analytics(bool threaded) {
	if (threaded) {
		Environment::thread_safe_function[8].BlockingCall([](Napi::Env env, Napi::Function function) {
			Napi::Object analytics_object = Napi::Object::New(Environment::context->Global().Env());
			analytics_object.Set("total_transactions", Analytics::total_transactions.load());
			analytics_object.Set("total_spent", Analytics::total_usd_spent.load());
			analytics_object.Set("spent_today", Analytics::total_usd_spent_today.load());
			function.Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_analytics"), analytics_object });
		});
	} else {
		Napi::Object analytics_object = Napi::Object::New(Environment::context->Global().Env());
		analytics_object.Set("total_transactions", Analytics::total_transactions.load());
		analytics_object.Set("total_spent", Analytics::total_usd_spent.load());
		analytics_object.Set("spent_today", Analytics::total_usd_spent_today.load());
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_analytics"), analytics_object });
	}
}

void Environment::main_update_total_value_spent(const Napi::CallbackInfo& info) {
	if (info[0].ToString().Utf8Value() == "ETH") {
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_total_value_spent"), Napi::Number::New(Environment::context->Global().Env(), Analytics::total_ethereum_spent) });
	} else if (info[0].ToString().Utf8Value() == "SOL") {
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_total_value_spent"),  Napi::Number::New(Environment::context->Global().Env(), Analytics::total_solana_spent) });
	}
}

void Environment::main_update_todays_transactions(bool threaded) {
	if (threaded) {
		Environment::thread_safe_function[8].BlockingCall([](Napi::Env env, Napi::Function function) {
			Napi::Array todays_transactions_array = Napi::Array::New(Environment::context->Global().Env());
			for (int x = 0; x < Module::todays_transactions.size(); x++) {
				Napi::Object transaction_object = Napi::Object::New(Environment::context->Global().Env());
				transaction_object.Set("blockchain", Module::todays_transactions[x].blockchain);
				transaction_object.Set("transaction_hash", Module::todays_transactions[x].transaction_hash);
				transaction_object.Set("date_initiated", Module::todays_transactions[x].date_initiated);
				todays_transactions_array.Set(uint32_t(x), transaction_object);
			}
			function.Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_todays_transactions"), todays_transactions_array });
		});
	} else {
		Napi::Array todays_transactions_array = Napi::Array::New(Environment::context->Global().Env());
		for (int x = 0; x < Module::todays_transactions.size(); x++) {
			Napi::Object transaction_object = Napi::Object::New(Environment::context->Global().Env());
			transaction_object.Set("blockchain", Module::todays_transactions[x].blockchain);
			transaction_object.Set("transaction_hash", Module::todays_transactions[x].transaction_hash);
			transaction_object.Set("date_initiated", Module::todays_transactions[x].date_initiated);
			todays_transactions_array.Set(uint32_t(x), transaction_object);
		}
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_todays_transactions"), todays_transactions_array });
	}
}

void Environment::main_update_contract_tasks(bool threaded) {
	if (threaded) {
		Environment::thread_safe_function[8].BlockingCall([](Napi::Env env, Napi::Function function) {
			Napi::Array contract_tasks_array = Napi::Array::New(Environment::context->Global().Env());
			for (int x = 0; x < Module::contract_tasks_.size(); x++) {
				Napi::Object contract_task_object = Napi::Object::New(Environment::context->Global().Env());
				contract_task_object.Set("task_id", Module::contract_tasks_[x]->task_id);
				contract_task_object.Set("blockchain", Module::contract_tasks_[x]->blockchain);
				if (Module::contract_tasks_[x]->blockchain == "Ethereum") {
					contract_task_object.Set("contract_address", Module::contract_tasks_[x]->ethereum_contract_address);
				} else if (Module::contract_tasks_[x]->blockchain == "Solana") {
					contract_task_object.Set("contract_address", Module::contract_tasks_[x]->solana_candy_machine_id);
				}
				contract_task_object.Set("wallet_group_name", Module::contract_tasks_[x]->wallet_group.name);
				contract_task_object.Set("time_started", Module::contract_tasks_[x]->time_started);
				contract_tasks_array.Set(uint32_t(x), contract_task_object);
			}
			function.Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_contract_tasks"), contract_tasks_array });
		});
	} else {
		Napi::Array contract_tasks_array = Napi::Array::New(Environment::context->Global().Env());
		for (int x = 0; x < Module::contract_tasks_.size(); x++) {
			Napi::Object contract_task_object = Napi::Object::New(Environment::context->Global().Env());
			contract_task_object.Set("task_id", Module::contract_tasks_[x]->task_id);
			contract_task_object.Set("blockchain", Module::contract_tasks_[x]->blockchain);
			if (Module::contract_tasks_[x]->blockchain == "Ethereum") {
				contract_task_object.Set("contract_address", Module::contract_tasks_[x]->ethereum_contract_address);
			} else if (Module::contract_tasks_[x]->blockchain == "Solana") {
				contract_task_object.Set("contract_address", Module::contract_tasks_[x]->solana_candy_machine_id);
			}
			contract_task_object.Set("wallet_group_name", Module::contract_tasks_[x]->wallet_group.name);
			contract_task_object.Set("time_started", Module::contract_tasks_[x]->time_started);
			contract_tasks_array.Set(uint32_t(x), contract_task_object);
		}
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_contract_tasks"), contract_tasks_array });
	}
}

void Environment::main_update_sniper_tasks(bool threaded) {
	if (threaded) {
		Environment::thread_safe_function[8].BlockingCall([](Napi::Env env, Napi::Function function) {
			Napi::Array sniper_tasks_array = Napi::Array::New(Environment::context->Global().Env());
			for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
				Napi::Object sniper_task_object = Napi::Object::New(Environment::context->Global().Env());
				sniper_task_object.Set("task_id", Module::sniper_tasks_[x]->task_id);
				sniper_task_object.Set("blockchain", Module::sniper_tasks_[x]->blockchain);
				sniper_task_object.Set("marketplace", Module::sniper_tasks_[x]->marketplace);
				sniper_task_object.Set("collection_slug", Module::sniper_tasks_[x]->collection);
				sniper_task_object.Set("maximum_value", Module::sniper_tasks_[x]->maximum_value);
				sniper_task_object.Set("minimum_value", Module::sniper_tasks_[x]->minimum_value);
				sniper_tasks_array.Set(uint32_t(x), sniper_task_object);
			}
			function.Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_sniper_tasks"), sniper_tasks_array });
		});
	} else {
		Napi::Array sniper_tasks_array = Napi::Array::New(Environment::context->Global().Env());
		for (int x = 0; x < Module::sniper_tasks_.size(); x++) {
			Napi::Object sniper_task_object = Napi::Object::New(Environment::context->Global().Env());
			sniper_task_object.Set("task_id", Module::sniper_tasks_[x]->task_id);
			sniper_task_object.Set("blockchain", Module::sniper_tasks_[x]->blockchain);
			sniper_task_object.Set("marketplace", Module::sniper_tasks_[x]->marketplace);
			sniper_task_object.Set("collection_slug", Module::sniper_tasks_[x]->collection);
			sniper_task_object.Set("maximum_value", Module::sniper_tasks_[x]->maximum_value);
			sniper_task_object.Set("minimum_value", Module::sniper_tasks_[x]->minimum_value);
			sniper_tasks_array.Set(uint32_t(x), sniper_task_object);
		}
		Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_sniper_tasks"), sniper_tasks_array });
	}
}

void Environment::main_update_wallet_groups() {
	Napi::Array wallet_group_array = Napi::Array::New(Environment::context->Global().Env());
	for (int x = 0; x < Module::wallet_groups.size(); x++) {
		Napi::Object wallet_group_object = Napi::Object::New(Environment::context->Global().Env());
		wallet_group_object.Set("name", Module::wallet_groups[x].name);
		wallet_group_object.Set("blockchain", Module::wallet_groups[x].blockchain);
		wallet_group_object.Set("private_keys", Module::wallet_groups[x].private_keys.size());
		wallet_group_object.Set("value", Wallet_Group::info[x].value);
		wallet_group_object.Set("acronym", Wallet_Group::info[x].acronym);
		wallet_group_object.Set("market_value", Wallet_Group::info[x].market_value);
		wallet_group_array.Set(uint32_t(x), wallet_group_object);
	}
	Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_wallet_groups"), wallet_group_array });
}

void Environment::main_update_transaction_history(bool threaded, bool insert) {
	if (threaded) {
		Environment::thread_safe_function[8].BlockingCall([insert](Napi::Env env, Napi::Function function) {
			if (!insert) {
				int x = 0;
				Napi::Array transaction_history_array = Napi::Array::New(Environment::context->Global().Env());
				BOOST_FOREACH(ptree::value_type & instance, Module::transaction_history.get_child("Transaction_Library")) {
					assert(instance.first.empty());
					Napi::Object transaction_object = Napi::Object::New(Environment::context->Global().Env());
					transaction_object.Set("blockchain", instance.second.get<string>("Blockchain"));
					transaction_object.Set("transaction_hash", instance.second.get<string>("Transaction_Hash"));
					transaction_object.Set("date_initiated", instance.second.get<int64_t>("Date_Initiated"));
					transaction_history_array.Set(uint32_t(x), transaction_object);
					x++;
				}
				function.Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_transaction_history"), transaction_history_array });
			} else {
				Transaction temp_txn = Module::todays_transactions[Module::todays_transactions.size() - 1];
				Napi::Object transaction_object = Napi::Object::New(Environment::context->Global().Env());
				transaction_object.Set("blockchain", temp_txn.blockchain);
				transaction_object.Set("transaction_hash", temp_txn.transaction_hash);
				transaction_object.Set("date_initiated", temp_txn.date_initiated);
				function.Call({ Napi::String::New(Environment::context->Global().Env(), "main_insert_transaction_history"), transaction_object });
			}
		});
	} else {
		if (!insert) {
			int x = 0;
			Napi::Array transaction_history_array = Napi::Array::New(Environment::context->Global().Env());
			BOOST_FOREACH(ptree::value_type & instance, Module::transaction_history.get_child("Transaction_Library")) {
				assert(instance.first.empty());
				Napi::Object transaction_object = Napi::Object::New(Environment::context->Global().Env());
				transaction_object.Set("blockchain", instance.second.get<string>("Blockchain"));
				transaction_object.Set("transaction_hash", instance.second.get<string>("Transaction_Hash"));
				transaction_object.Set("date_initiated", instance.second.get<int64_t>("Date_Initiated"));
				transaction_history_array.Set(uint32_t(x), transaction_object);
				x++;
			}
			Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_update_transaction_history"), transaction_history_array });
		} else {
			Transaction temp_txn = Module::todays_transactions[Module::todays_transactions.size() - 1];
			Napi::Object transaction_object = Napi::Object::New(Environment::context->Global().Env());
			transaction_object.Set("blockchain", temp_txn.blockchain);
			transaction_object.Set("transaction_hash", temp_txn.transaction_hash);
			transaction_object.Set("date_initiated", temp_txn.date_initiated);
			Environment::function_reference[8].Call({ Napi::String::New(Environment::context->Global().Env(), "main_insert_transaction_history"), transaction_object });
		}
	}
}

void Environment::client_create_public_variable_poll_head(int task_id, string https_url, int chain_id, string contract_address, string variable_name, int value, int timeout) {
	Napi::Number _task_id = Napi::Number::New(Environment::context->Global().Env(), task_id);
	Napi::String _https_url = Napi::String::New(Environment::context->Global().Env(), https_url);
	Napi::Number _chain_id = Napi::Number::New(Environment::context->Global().Env(), chain_id);
	Napi::String _contract_address = Napi::String::New(Environment::context->Global().Env(), contract_address);
	Napi::String _variable_name = Napi::String::New(Environment::context->Global().Env(), variable_name);
	Napi::Number _value = Napi::Number::New(Environment::context->Global().Env(), value);
	Napi::Number _timeout = Napi::Number::New(Environment::context->Global().Env(), timeout);
	Environment::function_reference[14].Call({ _task_id, _https_url, _chain_id, _contract_address, _variable_name, _value, _timeout });
}

void Environment::client_create_pending_transaction_poll_head(int task_id, string wss_url, int chain_id, string contract_address, string owner_address, string function_name, int timeout) {
	Napi::Number _task_id = Napi::Number::New(Environment::context->Global().Env(), task_id);
	Napi::String _wss_url = Napi::String::New(Environment::context->Global().Env(), wss_url);
	Napi::Number _chain_id = Napi::Number::New(Environment::context->Global().Env(), chain_id);
	Napi::String _contract_address = Napi::String::New(Environment::context->Global().Env(), contract_address);
	Napi::String _owner_address = Napi::String::New(Environment::context->Global().Env(), owner_address);
	Napi::String _function_name = Napi::String::New(Environment::context->Global().Env(), function_name);
	Napi::Number _timeout = Napi::Number::New(Environment::context->Global().Env(), timeout);
	Environment::function_reference[15].Call({ _task_id, _wss_url, _chain_id, _contract_address, _owner_address, _function_name, _timeout });
}

void Environment::client_poll_ethereum_transaction(string txn_hash, string https_url, int chain_id, string discord_webhook) {
	Environment::thread_safe_function[16].BlockingCall([txn_hash, https_url, chain_id, discord_webhook](Napi::Env env, Napi::Function function) {
		Napi::String _txn_hash = Napi::String::New(Environment::context->Global().Env(), txn_hash);
		Napi::String _https_url = Napi::String::New(Environment::context->Global().Env(), https_url);
		Napi::Number _chain_id = Napi::Number::New(Environment::context->Global().Env(), chain_id);
		Napi::String _discord_webhook = Napi::String::New(Environment::context->Global().Env(), discord_webhook);
		function.Call({ _txn_hash, _https_url, _chain_id, _discord_webhook });
	});
}

void Environment::candy_machine_poll(int task_id, string rpc_url, string address, string type) {
	Napi::Number _task_id = Napi::Number::New(Environment::context->Global().Env(), task_id);
	Napi::String _rpc_url = Napi::String::New(Environment::context->Global().Env(), rpc_url);
	Napi::String _address = Napi::String::New(Environment::context->Global().Env(), address);
	Napi::String _type = Napi::String::New(Environment::context->Global().Env(), type);
	Environment::function_reference[17].Call({ _task_id, _rpc_url, _address, _type });
}