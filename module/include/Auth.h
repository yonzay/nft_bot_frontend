#pragma once
#include <string>
#include <atomic>
#include <fstream>

using namespace std;

class Auth {
private:
	static const char* host;
	static const char* port;
public:
	static atomic<bool> auto_login;
	static string current_session;
	static string discord_username;
	static atomic<int> nft_iteration;
	static string nft_image_url;
	static string quote;
	static string email_address;
	static int64_t renewal_date;
	static int64_t last_login;
	static string wallet_address;
	static atomic<bool> is_logged_in;
	static string hyper_auth_cert;
	static void login();
	static void load_hyper_auth_cert();
};