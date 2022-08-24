#include "Contract_Task.h"

const char* Contract_Task::host = "blockchain-service-1859773842.us-east-1.elb.amazonaws.com";
const char* Contract_Task::port = "80";

boost::mutex Contract_Task::control;
vector<thread_id> Contract_Task::active_contract_threads;
vector<atomic<int>> Contract_Task::stop_pool(100);
vector<atomic<int>> Contract_Task::delete_pool(100);