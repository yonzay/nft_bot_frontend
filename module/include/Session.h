#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include "Module.h"
#include "Auth.h"
#include "Instance_Summary.h"
#include "Chronicle.h"
#include "Environment.h"
#include "Action.h"

#define NULL_STRING ""

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
    static const char* host;
    static const char* port;
    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string target_;
public:
    explicit Session(net::io_context& ioc) : resolver_(net::make_strand(ioc)), ws_(net::make_strand(ioc)) { }

    void run(char const* host, char const* port, char const* target) {
        host_ = host;
        target_ = target;
        resolver_.async_resolve(host, port, beast::bind_front_handler(&Session::on_resolve, shared_from_this()));
        Chronicle::print(true, type::message, false, "Attempting to establish a connection to the [MoonDweller-Service]...");
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) {
            Chronicle::print(true, type::error, false, ec.message());
            Chronicle::print(true, type::warning, false, "Closing in 5 seconds...");
            boost::this_thread::sleep_for(boost::chrono::seconds(10));
            Environment::thread_safe_function[9].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
            return;
        }
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
        beast::get_lowest_layer(ws_).async_connect(results, beast::bind_front_handler(&Session::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
        if (ec) {
            Chronicle::print(true, type::error, false, ec.message());
            Chronicle::print(true, type::warning, false, "Closing in 5 seconds...");
            boost::this_thread::sleep_for(boost::chrono::seconds(10));
            Environment::thread_safe_function[9].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
            return;
        }
        beast::get_lowest_layer(ws_).expires_never();
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws_.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
            req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
        }));
        host_ += ':' + std::to_string(ep.port());
        ws_.async_handshake(host_, target_, beast::bind_front_handler(&Session::on_handshake, shared_from_this()));
        Chronicle::print(true, type::message, false, "Successfully connected to the [MoonDweller-Service].");
    }

    void on_handshake(beast::error_code ec) {
        if (ec) {
            Chronicle::print(true, type::error, false, ec.message());
            Chronicle::print(true, type::warning, false, "Closing in 5 seconds...");
            boost::this_thread::sleep_for(boost::chrono::seconds(10));
            Environment::thread_safe_function[9].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
            return;
        }
        ws_.async_read(buffer_, beast::bind_front_handler(&Session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec) {
            Chronicle::print(true, type::error, false, ec.message());
            Chronicle::print(true, type::warning, false, "Closing in 5 seconds...");
            boost::this_thread::sleep_for(boost::chrono::seconds(10));
            Environment::thread_safe_function[9].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
            return;
        }
        stringstream buffer_stream;
        buffer_stream << beast::make_printable(buffer_.data());
        ptree json;
        read_json(buffer_stream, json);
        if (json.get<string>("response_type") == "new_connection") {
            Auth::current_session = json.get<string>("session_id");
            Chronicle::print(true, type::message, false, "Successfully obtained custom session.");
            Auth::quote = json.get<string>("dashboard_quote");
            Instance_Summary::current_gas = json.get<int>("current_fast_gas_price");
            Instance_Summary::ethereum_value = json.get<double>("current_eth_market_value");
            Instance_Summary::solana_value = json.get<double>("current_sol_market_value");
            Environment::main_update_instance_summary(true);
            buffer_.consume(buffer_.size());
            if (Auth::auto_login) {
                Auth::login();
            } else {
                Environment::thread_safe_function[2].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
            }
        } else if (json.get<string>("response_type") == "information") {
            Instance_Summary::current_gas = json.get<int>("current_fast_gas_price");
            Instance_Summary::ethereum_value = json.get<double>("current_eth_market_value");
            Instance_Summary::solana_value = json.get<double>("current_sol_market_value");
            Environment::main_update_instance_summary(true);
            buffer_.consume(buffer_.size());
        }
        boost::this_thread::sleep_for(boost::chrono::milliseconds(60000));
        ws_.async_ping(NULL_STRING, beast::bind_front_handler(&Session::on_pong, shared_from_this()));
    }

    void on_pong(beast::error_code ec) {
        if (ec) {
            Chronicle::print(true, type::error, false, ec.message());
            Chronicle::print(true, type::warning, false, "Closing in 5 seconds...");
            boost::this_thread::sleep_for(boost::chrono::seconds(10));
            Environment::thread_safe_function[9].BlockingCall([](Napi::Env env, Napi::Function function) { function.Call({}); });
            return;
        }
        ws_.async_read(buffer_, beast::bind_front_handler(&Session::on_read, shared_from_this()));
    }

    static void new_session();
};