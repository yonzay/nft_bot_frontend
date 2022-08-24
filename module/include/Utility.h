#pragma once
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;

class Utility {
public:
	static int mod(int a, int b);
	static string format_json(ptree json);
	static bool file_exists(const string& name);
};