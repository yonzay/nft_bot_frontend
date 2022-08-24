#include "Sniper_task.h"

const char* Sniper_Task::host = "blockchain-service-1859773842.us-east-1.elb.amazonaws.com";
const char* Sniper_Task::port = "80";

vector<atomic<int>> Sniper_Task::stop_pool(100);
vector<atomic<int>> Sniper_Task::delete_pool(100);