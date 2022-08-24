#pragma once
#include <string>
#include <chrono>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>
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

class Sniper_Task {
	static const char* host;
	static const char* port;
public:
	int task_id;
	string blockchain;
	string marketplace;
	string marketplace_api_key;
	string collection;
	string wallet_private_key;
	bool use_flashbots;
	string erc_20_token_address;
	int ethereum_gas_price;
	int ethereum_gas_limit;
	int ethereum_priority_fee;
	int ethereum_nonce;
	float minimum_value;
	float maximum_value;
	int delay;
	static vector<atomic<int>> stop_pool;
	static vector<atomic<int>> delete_pool;
	void instance(int iteration) {
		ptree base;
		ptree encrypted;
		ptree response;
		float starting_value;
		string api;
		ptree container;
		if (this->blockchain == "Ethereum") {
			api = "ethers";
			base.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
			base.put("ethereum.chain_id", Settings::ethereum_chain_id);
			if (this->use_flashbots) { base.put("ethereum.flashbots_signer", Settings::flashbots_signer); }
			container.put("ethereum.provider_api_url", Settings::ethereum_https_provider_api_url);
			container.put("ethereum.chain_id", Settings::ethereum_chain_id);
		} else if (this->blockchain == "Solana") {
			api = "solana";
			if (Settings::solana_rpc_urls.size() > 0) {
				base.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
				container.put("solana.rpc_url", Settings::solana_rpc_urls[0]);
			}
		}
		ptree private_key_container;
		private_key_container.push_back(std::make_pair("", ptree(("", this->wallet_private_key))));
		encrypted.put("session", Auth::current_session);
		container.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
		container.put_child("private_keys", private_key_container);
		encrypted.clear();
		response = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + api + "/wallet_group_balance").c_str(), Utility::format_json(container).c_str(), "text/plain");
        if (response.size() == 0) {
            Chronicle::print(true, type::error, false, "Failed to track requested key, [" + this->blockchain + "] spent will not tracked accurately.");
        } else {
            if (!response.get<bool>("success")) {
                Chronicle::print(true, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
            } else if (response.get<bool>("success")) {
				starting_value = response.get<float>("value");
            }
        }
		encrypted.put("session", Auth::current_session);
		encrypted.put("wallet_private_key", this->wallet_private_key);
		base.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
		base.put("marketplace", this->marketplace);
		base.put("marketplace_api_key", this->marketplace_api_key);
		base.put("collection", this->collection);
		if (this->blockchain == "Ethereum") {
			base.put("ethereum.use_flashbots", this->use_flashbots);
			base.put("ethereum.erc_20_token_address", this->erc_20_token_address);
			base.put("ethereum.gas_price", this->ethereum_gas_price);
			base.put("ethereum.gas_limit", this->ethereum_gas_limit);
			base.put("ethereum.priority_fee", this->ethereum_priority_fee);
			base.put("nonce", this->ethereum_nonce);
		} else if (this->blockchain == "Solana") {

		}
		base.put("minimum_value", this->minimum_value);
		base.put("maximum_value", this->maximum_value);
		encrypted.clear();
		if (this->delay < 1000) { this->delay = 1000; }
		while (true) {
			for (int x = 0; x < this->stop_pool.size(); x++) {
				if (this->task_id == this->stop_pool[x]) {
					while (this->task_id == this->stop_pool[x]) {
						boost::this_thread::sleep_for(boost::chrono::seconds(1));
					}
				}
			}
			for (int x = 0; x < this->delete_pool.size(); x++) {
				if (this->task_id == this->delete_pool[x]) {
					float ending_value;
					encrypted.put("session", Auth::current_session);
					container.put("encrypted", Cryptography::rsa_encrypt(Utility::format_json(encrypted)));
					container.put_child("private_keys", private_key_container);
					encrypted.clear();
					response = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + api + "/wallet_group_balance").c_str(), Utility::format_json(container).c_str(), "text/plain");
					if (response.size() == 0) {
						Chronicle::print(true, type::error, false, "Failed to track requested key, [" + this->blockchain + "] spent will not tracked accurately.");
					} else {
						if (!response.get<bool>("success")) {
							Chronicle::print(true, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
						} else if (response.get<bool>("success")) {
							ending_value = response.get<float>("value");
						}
					}
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
					for (int x = 0; x < this->delete_pool.size(); x++) {
						if (this->delete_pool[x] == this->task_id) { this->delete_pool[x] = 0; }
					}
					Chronicle::print(true, type::message, false, "Successfully deleted [Sniper-Task] [" + to_string(this->task_id) + "].");
					Action::remove_sniper_task(this->task_id);
					return;
				}
			}
			std::srand(time(0));
			if (this->marketplace != "magiceden") {
				if (Settings::loaded_proxies.size() > 0) {
					base.put("proxy", Settings::loaded_proxies[Utility::mod(rand() % (Settings::loaded_proxies.size() - 0) + 0, Settings::loaded_proxies.size())]);
				}
				response = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + this->marketplace + "/collection_sniper").c_str(), Utility::format_json(base).c_str(), "text/plain");
				if (response.size() == 0) {
					Chronicle::print(true, type::error, false, "Request Failed");
				} else {
					if (!response.get<bool>("success")) {
						Chronicle::print(true, type::error, false, response.get_optional<string>("error").get_value_or(response.get_optional<string>("reason").get_value_or("null")));
					} else if (response.get<bool>("success")) {
						BOOST_FOREACH(ptree::value_type & instance, response.get_child("buy_now")) {
							assert(instance.first.empty());
							if (this->blockchain == "Ethereum") {
								base.put("token_id", instance.second.get<string>("token_id"));
								base.put("ethereum.contract_address", instance.second.get<string>("contract_address"));
								base.put("token_standard", instance.second.get<string>("token_standard"));
								base.put("ethereum.value", instance.second.get<string>("value"));
								ptree response_two = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + this->marketplace + "/atomic_match").c_str(), Utility::format_json(base).c_str(), "text/plain");
								if (response_two.size() == 0) {
									Chronicle::print(true, type::error, false, "Request Failed");
								} else {
									if (!response_two.get_optional<bool>("success").get_value_or(false)) {
										Chronicle::print(true, type::error, false, response_two.get_optional<string>("error").get_value_or(response_two.get_optional<string>("reason").get_value_or("null")));
									} else if (response_two.get<bool>("success")) {
										Action::add_transaction(true, this->blockchain, response_two.get<string>("txn_hash"));
									}
								}
							}
						}
					}
				}
			} else if (this->marketplace == "magiceden") {
				response = Http_Request::send_https(verb::get, "api-mainnet.magiceden.io", "443", ("/rpc/getListedNFTsByQuery?q=%7B%22%24match%22%3A%7B%22collectionSymbol%22%3A%22" + this->collection + "%22%7D%2C%22%24sort%22%3A%7B%22createdAt%22%3A-1%7D%2C%22%24skip%22%3A0%2C%22%24limit%22%3A20%7D").c_str(), NULL, NULL);
				if (response.size() == 0) {
					Chronicle::print(true, type::error, false, "Request Failed");
				} else {
					try {
						BOOST_FOREACH(ptree::value_type & instance, response.get_child("results")) {
							assert(instance.first.empty());
							float price = instance.second.get<float>("price");
							if (price != 0 && price >= this->minimum_value && price <= this->maximum_value) {
								base.put("solana.token_mint_address", instance.second.get<string>("mintAddress"));
								base.put("solana.token_seller_address", instance.second.get<string>("owner"));
								base.put("solana.token_price", price);
								ptree response_two = Http_Request::send(verb::post, this->host, this->port, ("/api/v4/user/" + this->marketplace + "/atomic_match").c_str(), Utility::format_json(base).c_str(), "text/plain");
								if (response_two.size() == 0) {
									Chronicle::print(true, type::error, false, "Request Failed");
								} else {
									if (!response_two.get_optional<bool>("success").get_value_or(false)) {
										Chronicle::print(true, type::error, false, response_two.get_optional<string>("error").get_value_or(response_two.get_optional<string>("reason").get_value_or("null")));
									} else if (response_two.get<bool>("success")) {
										Action::add_transaction(true, this->blockchain, response_two.get<string>("txn_hash"));
									}
								}
							}
						}
					} catch (exception const& e) {
						Chronicle::print(true, type::error, false, "bad_data");
					}
				}
			}
			boost::this_thread::sleep_for(boost::chrono::milliseconds(this->delay));
		}
	};
};