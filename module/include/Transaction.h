#pragma once
#include <string>

using namespace std;

class Transaction {
public:
	string blockchain;
	string transaction_hash;
	int64_t date_initiated;
};