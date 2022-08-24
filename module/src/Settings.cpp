#include "Settings.h"

string Settings::ethereum_https_provider_api_url;
string Settings::ethereum_websocket_provider_api_url;
vector<string> Settings::solana_rpc_urls;
atomic<int> Settings::ethereum_chain_id;
atomic<bool> Settings::use_flashbots = false;
string Settings::flashbots_signer;
string Settings::discord_webhook;
string Settings::path_to_proxies;
vector<string> Settings::loaded_proxies;
ptree Settings::settings_container;