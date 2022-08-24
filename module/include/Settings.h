#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost::property_tree;

class Settings {
public:
	static string ethereum_https_provider_api_url;
	static string ethereum_websocket_provider_api_url;
	static vector<string> solana_rpc_urls;
	static atomic<int> ethereum_chain_id;
	static atomic<bool> use_flashbots;
	static string flashbots_signer;
	static string discord_webhook;
	static string path_to_proxies;
	static vector<string> loaded_proxies;
	static ptree settings_container;
};