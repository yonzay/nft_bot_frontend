#pragma once
#include <string>
#include <boost/beast/core.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/http.hpp>

using namespace boost::property_tree;
using namespace boost::beast::http;
using namespace std;

class Http_Request {
public:
    static ptree send(verb method, char const* host, char const* port, char const* target, char const* body, char const* content_type);
    static ptree send_https(verb method, char const* host, char const* port, char const* target, char const* body, char const* content_type);
};