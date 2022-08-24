#include "Analytics.h"

atomic<int> Analytics::total_transactions = 0;
atomic<int> Analytics::total_usd_spent = 0;
atomic<int> Analytics::total_usd_spent_today = 0;
atomic<float> Analytics::total_ethereum_spent = 0;
atomic<float> Analytics::total_ethereum_spent_today = 0;
atomic<float> Analytics::total_solana_spent = 0;
atomic<float> Analytics::total_solana_spent_today = 0;
atomic<int64_t> Analytics::from_to_now = 0;
ptree Analytics::analytics_container;