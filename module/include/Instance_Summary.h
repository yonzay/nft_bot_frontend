#pragma once
#include <string>
#include <atomic>

using namespace std;

class Instance_Summary {
public:
	static atomic<float> ethereum_value;
	static atomic<float> solana_value;
	static atomic<int> current_gas;
	static atomic<int> txns_attempted;
	static atomic<float> ethereum_spent;
	static atomic<float> solana_spent;
	static atomic<int> total_contract_tasks;
	static atomic<int> contract_tasks_active;
	static atomic<int> total_sniper_tasks;
	static atomic<int> sniper_tasks_active;
};