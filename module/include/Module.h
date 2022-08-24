#pragma once
#include <vector>
#include <atomic>
#include <boost/property_tree/json_parser.hpp>
#include "Contract_Task.h"
#include "Sniper_Task.h"
#include "Wallet_Group.h"
#include "Transaction.h"

using namespace std;
using namespace boost::property_tree;

class Module {
public:
	static string persistent_path;
	static string client_version;
	static atomic<bool> up_to_date;
	static vector<unique_ptr<Contract_Task>> contract_tasks_;
	static ptree contract_tasks_container;
	static vector<unique_ptr<Sniper_Task>> sniper_tasks_;
	static ptree sniper_tasks_container;
	static vector<Wallet_Group> wallet_groups;
	static ptree wallet_groups_container;
	static vector<Transaction> todays_transactions;
	static ptree transaction_history;
};