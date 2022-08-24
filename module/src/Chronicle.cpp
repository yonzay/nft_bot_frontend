#define WIN32_LEAN_AND_MEAN
#include <chrono>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <boost/asio/ip/host_name.hpp>
#include <boost/algorithm/string.hpp>
#include "Chronicle.h"
#include "Environment.h"
#include "Cryptography.h"
#include "Module.h"

string Chronicle::user = boost::algorithm::to_lower_copy(boost::asio::ip::host_name());
string Chronicle::session = Chronicle::get_session();

string Chronicle::get_session() {
	auto end = chrono::system_clock::now();
	time_t end_time = chrono::system_clock::to_time_t(end);
	string time_string = ctime(&end_time);
	time_string.erase(std::remove(time_string.begin(), time_string.end(), '\n'), time_string.end());
	do {
		size_t clean = time_string.find(":");
		time_string.replace(clean, 1, " ");
	} while (time_string.find(":") != string::npos);
	return "Chronicle " + time_string;
}

void Chronicle::print(bool threaded, int type, bool encrypt, string input) {
	ofstream fs;
	stringstream entry;
	auto end = chrono::system_clock::now();
	time_t end_time = chrono::system_clock::to_time_t(end);
	string time_string = ctime(&end_time);
	time_string.erase(std::remove(time_string.begin(), time_string.end(), '\n'), time_string.end());
	time_string = time_string.substr(11, 8);
	switch (type) {
	case type::message:
		Environment::chronicle_insert_log(threaded, "Message", input);
		break;
	case type::warning:
		Environment::chronicle_insert_log(threaded, "Warning", input);
		break;
	case type::error:
		Environment::chronicle_insert_log(threaded, "Error", input);
		break;
	}
	if (!encrypt) {
		entry << time_string + ' ' + input;
	} else {
		entry << time_string + ' ' + Cryptography::aes_encrypt(input);
	}
	fs.open(Module::persistent_path + "Chronicles/" + Chronicle::session + ".txt", ofstream::app);
	fs << entry.str() + '\n';
	fs.close();
}