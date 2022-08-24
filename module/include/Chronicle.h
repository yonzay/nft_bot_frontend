#pragma once
#include <string>
#include <vector>

using namespace std;

enum type {
	message,
	warning,
	error
};

class Chronicle {
public:
	static string user;
	static string session;
	static string get_session();
	static void print(bool threaded, int type, bool encrypt, string value);
};