#include "Instance_Summary.h"

atomic<float> Instance_Summary::ethereum_value = 0;
atomic<float> Instance_Summary::solana_value = 0;
atomic<int> Instance_Summary::current_gas = 0;
atomic<int> Instance_Summary::txns_attempted = 0;
atomic<float> Instance_Summary::ethereum_spent = 0;
atomic<float> Instance_Summary::solana_spent = 0;
atomic<int> Instance_Summary::total_contract_tasks = 0;
atomic<int> Instance_Summary::contract_tasks_active = 0;
atomic<int> Instance_Summary::total_sniper_tasks = 0;
atomic<int> Instance_Summary::sniper_tasks_active = 0;