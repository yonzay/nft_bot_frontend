#pragma once
#include <string>
#include <chrono>
#include <boost/thread/mutex.hpp>
#include "Auth.h"
#include "Action.h"
#include "Wallet_Group.h"
#include "Utility.h"
#include "Settings.h"
#include "Cryptography.h"
#include "Http_Request.h"
#include "Chronicle.h"
#include "Analytics.h"
#include "Instance_Summary.h"
#include "Environment.h"

using namespace std;
using namespace std::chrono;

struct thread_id {
	int task_id;
	int iteration;
};

class Contract_Task {
	static const char* host;
	static const char* port;
public:
	int task_id;
	string blockchain;
	Wallet_Group wallet_group;
	string ethereum_contract_address;
	string ethereum_function;
	ptree ethereum_call_data;
	bool is_polling;
	string ethereum_value;
	int ethereum_gas_price;
	int ethereum_gas_limit;
	int ethereum_priority_fee;
	int ethereum_amount_of_txns;
	string solana_mode;
	string solana_candy_machine_id;
	int solana_candy_machine_amount;
	int64_t time_started;
	static boost::mutex control;
	static vector<thread_id> active_contract_threads;
	static vector<atomic<int>> stop_pool;
	static vector<atomic<int>> delete_pool;
	void instance(int iteration) {
		thread_id this_thread;
		this_thread.task_id = this->task_id;
		this_thread.iteration = iteration;
		{
			boost::mutex::scoped_lock lock(this->control);
			this->active_contract_threads.push_back(this_thread);
		}
		ptree base;
		ptree encrypted;
		ptree response;
		string api;
		bool failed_fetch = false;
		double starting_value;
		ptree container;
		if (this->blockchain == "Ethereum") {
			api = "ethers";
			base.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
			container.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
			base.put("ethereum.chain_id", Settings::ethereum_chain_id);
			container.put("ethereum.chain_id", Settings::ethereum_chain_id);
			base.put("ethereum.use_flashbots", Settings::use_flashbots.load());
			base.put("ethereum.flashbots_signer", Settings::flashbots_signer);
		} else if (this->blockchain == "Solana") {
			api = "solana";
			if (Settings::solana_rpc_urls.size() > 0) {
				base.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
				container.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
			}
		}
		ptree starting_value_container;
		starting_value_container.push_back(std::make_pair("", ptree(("", this->wallet_group.private_keys[iteration]))));
		encrypted.put("session", Auth::current_session);
		container.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
		container.put_child("private_keys", starting_value_container);
		encrypted.clear();
		response = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + api + "/wallet_group_balance").c_str(), Utility::format_json(container).c_str(), "text/plain");
		if (response.size() == 0) {
			Chronicle::print(true, type::error, false, "Failed to track a key in [Wallet-Group] [" + this->wallet_group.name + "], [" + this->wallet_group.blockchain + "] spent will not be tracked accurately.");
			failed_fetch = true;
		} else {
			if (!response.get<bool>("success")) {
				Chronicle::print(true, type::error, false, "Failed to track a key in [Wallet-Group] [" + this->wallet_group.name + "], [" + this->wallet_group.blockchain + "] spent will not be tracked accurately.");
				Chronicle::print(true, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
				failed_fetch = true;
			} else if (response.get<bool>("success")) {
				starting_value = response.get<float>("value");
			}
		}
		container.clear();
		starting_value_container.clear();
		encrypted.put("session", Auth::current_session);
		encrypted.put("wallet_private_key", this->wallet_group.private_keys[iteration]);
		base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
		if (this->blockchain == "Ethereum") {
			base.put("ethereum.value", this->ethereum_value);
			base.put("ethereum.contract_address", this->ethereum_contract_address);
			base.put("ethereum.function", this->ethereum_function);
			base.put_child("ethereum.call_data", this->ethereum_call_data);
			base.put("ethereum.gas_price", this->ethereum_gas_price);
			base.put("ethereum.gas_limit", this->ethereum_gas_limit);
			base.put("ethereum.priority_fee", this->ethereum_priority_fee);
		} else if (this->blockchain == "Solana") {
			api = "candymachine";
			base.put("solana.mode", this->solana_mode);
			base.put("solana.candy_machine_id", this->solana_candy_machine_id);
			base.put("solana.candy_machine_amount", this->solana_candy_machine_amount);
		}
		encrypted.clear();
		int64_t nonce = 0;
		while (true) {
			if (this->blockchain == "Ethereum") {
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1500));
			}
			for (int x = 0; x < this->stop_pool.size(); x++) {
				if (this->task_id == this->stop_pool[x]) {
					while (this->task_id == this->stop_pool[x]) {
						boost::this_thread::sleep_for(boost::chrono::seconds(1));
					}
				}
			}
			for (int x = 0; x < this->delete_pool.size(); x++) {
				if (this->task_id == this->delete_pool[x]) {
					boost::mutex::scoped_lock lock(this->control);
					int count = 0;
					for (int y = 0; y < this->active_contract_threads.size(); y++) {
						if (this->task_id == this->active_contract_threads[y].task_id) {
							count++;
						}
					}
					if (count == 1) {
						for (int y = 0; y < this->active_contract_threads.size(); y++) {
							if (this->task_id == this->active_contract_threads[y].task_id) {
								if (!failed_fetch) {
									ptree container;
									ptree ending_value_container;
									float ending_value;
									container.put("", wallet_group.private_keys[iteration]);
									ending_value_container.push_back(std::make_pair("", container));
									container.clear();
									if (this->blockchain == "Ethereum") {
										container.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
										container.put("ethereum.chain_id", Settings::ethereum_chain_id);
									} else if (this->blockchain == "Solana") {
										api = "solana";
										if (Settings::solana_rpc_urls.size() > 0) {
											container.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
										}
									}
									encrypted.put("session", Auth::current_session);
									container.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
									container.put_child("private_keys", ending_value_container);
									encrypted.clear();
									response = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + api + "/wallet_group_balance").c_str(), Utility::format_json(container).c_str(), "text/plain");
									if (response.size() == 0) {
										Chronicle::print(true, type::error, false, "Failed to track a key in [Wallet-Group] [" + this->wallet_group.name + "], [" + this->wallet_group.blockchain + "] spent will not be tracked accurately.");
									} else {
										if (!response.get<bool>("success")) {
											Chronicle::print(true, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
										} else if (response.get<bool>("success")) {
											ending_value = response.get<double>("value");
										}
									}
									container.clear();
									ending_value_container.clear();
									if (this->blockchain == "Ethereum") {
										Analytics::total_ethereum_spent = Analytics::total_ethereum_spent + (starting_value - ending_value);
										Analytics::total_ethereum_spent_today = Analytics::total_ethereum_spent_today + (starting_value - ending_value);
										Instance_Summary::ethereum_spent = Instance_Summary::ethereum_spent + (starting_value - ending_value);
									} else if (this->blockchain == "Solana") {
										Analytics::total_solana_spent = Analytics::total_solana_spent + (starting_value - ending_value);
										Analytics::total_solana_spent_today = Analytics::total_solana_spent_today + (starting_value - ending_value);
										Instance_Summary::solana_spent = Instance_Summary::solana_spent + (starting_value - ending_value);
									}
									Action::update_analytics(true);
									Environment::main_update_instance_summary(true);
								}
								this->active_contract_threads.erase(this->active_contract_threads.begin() + y);
								for (int x = 0; x < this->delete_pool.size(); x++) {
									if (this->delete_pool[x] == this->task_id) { this->delete_pool[x] = 0; }
								}
								Chronicle::print(true, type::message, false, "Successfully deleted [Contract-Task] [" + to_string(this->task_id) + "].");
								Action::remove_contract_task(this->task_id);
								return;
							}
						}
					} else {
						for (int y = 0; y < this->active_contract_threads.size(); y++) {
							if (this->task_id == this->active_contract_threads[y].task_id) {
								this->active_contract_threads.erase(this->active_contract_threads.begin() + y);
								Chronicle::print(true, type::message, false, "Successfully ended [Thread] [" + to_string(this->active_contract_threads[y].iteration) + "] of [Contract-Task] [" + to_string(this->active_contract_threads[y].task_id) + "].");
								return;
							}
						}
					}
				}
			}
			if (this->blockchain == "Ethereum") {
				base.put("nonce", nonce);
			} else if (this->blockchain == "Solana") {
				if (Settings::solana_rpc_urls.size() > 0) {
					base.put("solana.rpc_url", Settings::solana_rpc_urls[Utility::mod(nonce, Settings::solana_rpc_urls.size())]);
				}
			}
			response = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + api + "/execute_function").c_str(), Utility::format_json(base).c_str(), "text/plain");
			if (response.size() == 0) {
				Chronicle::print(true, type::error, false, "Request Failed");
			} else {
				if (!response.get<bool>("success")) {
					Chronicle::print(true, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
				} else if (response.get<bool>("success")) {
					boost::mutex::scoped_lock lock(this->control);
					if (this->blockchain == "Ethereum") {
						Action::add_transaction(true, this->blockchain, response.get<string>("txn_hash"));
						if (this->ethereum_amount_of_txns == this->wallet_group.private_keys.size() - 1) {
							bool is_stopped = false;
							bool is_being_deleted = false;
							for (int x = 0; x < this->stop_pool.size(); x++) {
								if (this->stop_pool[x] == this->task_id) { is_stopped = true; }
							}
							for (int x = 0; x < this->delete_pool.size(); x++) {
								if (this->delete_pool[x] == this->task_id) { is_being_deleted = true; }
							}
							if (!is_stopped && !is_being_deleted) {
								for (int x = 0; x < this->stop_pool.size(); x++) {
									if (this->stop_pool[x] == 0) {
										this->stop_pool[x] = this->task_id;
										Instance_Summary::contract_tasks_active--;
										Environment::main_update_instance_summary(true);
										Chronicle::print(true, type::message, false, "Stopping [Contract-Task] [" + to_string(this->task_id) + "]... Resume to send a new batch of requests corresponding to this [Wallet-Group].");
										nonce++;
										break;
									}
								}
							}
							continue;
						}
						this->ethereum_amount_of_txns++;
					} else if (this->blockchain == "Solana") {
						Instance_Summary::txns_attempted++;
						Environment::main_update_instance_summary(true);
						Chronicle::print(true, type::message, false, "Attempted [Transaction] [" + response.get<string>("txn_hash") + "].");
					}
				}
			}
			nonce++;
		}
	};
};