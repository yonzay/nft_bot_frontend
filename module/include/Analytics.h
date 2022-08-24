#pragma once
#include <atomic>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;

class Analytics {
public:
	static atomic<int> total_transactions;
	static atomic<int> total_usd_spent;
	static atomic<int> total_usd_spent_today;
	static atomic<float> total_ethereum_spent;
	static atomic<float> total_ethereum_spent_today;
	static atomic<float> total_solana_spent;
	static atomic<float> total_solana_spent_today;
	static atomic<int64_t> from_to_now;
	static ptree analytics_container;
};