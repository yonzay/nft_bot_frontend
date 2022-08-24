#pragma once
#include <string>
#include <vector>

using namespace std;

class Wallet_Group {
public:
	string name;
	string blockchain;
	vector<string> private_keys;
	struct metadata {
		string name;
		float value;
		int market_value;
		string acronym;
	};
	static vector<metadata> info;
};