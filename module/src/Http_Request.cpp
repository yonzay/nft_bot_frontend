#include "Http_Request.h"
#include "Root_Certificates.h"

using tcp = boost::asio::ip::tcp;
using namespace boost::system::errc;
using namespace boost::asio;
using namespace boost::beast;
using namespace net::ssl;

typedef boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO> rcv_timeout_option;

ptree Http_Request::send(verb method, char const* host, char const* port, char const* target, char const* body, char const* content_type) {
    io_context ioc;
    tcp::resolver resolver{ ioc };
    tcp::socket socket{ ioc };
    flat_buffer buffer;
    request<string_body> request;
    response<string_body> response;
    stringstream container;
    ptree data;
    try {
        auto const results = resolver.resolve(host, port);
        connect(socket, results.begin(), results.end());
        request.method(method);
        request.target(target);
        request.set(field::host, host);
        request.set(field::user_agent, BOOST_BEAST_VERSION_STRING);
        if (method == verb::post) {
            request.body() = body;
            request.set(field::content_type, content_type);
            request.prepare_payload();
        }
        socket.set_option(rcv_timeout_option{ 30000 });
        write(socket, request);
        read(socket, buffer, response);
        boost::beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != not_connected) { throw boost::system::system_error{ ec }; }
        container << response.body();
        read_json(container, data);
        return data;
    } catch (exception const& e) {
        return data;
    }
}

ptree Http_Request::send_https(verb method, char const* host, char const* port, char const* target, char const* body, char const* content_type) {
    io_context ioc;
    context ctx(ssl::context::tlsv12_client);
    tcp::resolver resolver(ioc);
    ssl_stream<tcp_stream> stream(ioc, ctx);
    flat_buffer buffer;
    request<string_body> request;
    response<string_body> response;
    stringstream container;
    ptree data;
    try {
        load_root_certificates(ctx);
        ctx.set_verify_mode(ssl::verify_peer);
        SSL_set_cipher_list(stream.native_handle(), "AES128-GCM-SHA256");
        SSL_set_tlsext_host_name(stream.native_handle(), host);
        auto const results = resolver.resolve(host, port);
        get_lowest_layer(stream).connect(results);
        stream.handshake(ssl::stream_base::client);
        request.method(method);
        request.target(target);
        request.set(field::host, host);
        request.set(field::user_agent, BOOST_BEAST_VERSION_STRING);
        if (method == verb::post) {
            request.body() = body;
            request.set(field::content_type, content_type);
            request.prepare_payload();
        }
        get_lowest_layer(stream).socket().set_option(rcv_timeout_option{ 30000 });
        write(stream, request);
        read(stream, buffer, response);
        boost::beast::error_code ec;
        stream.shutdown(ec);
        container << response.body();
        read_json(container, data);
        return data;
    } catch (exception const& e) {
        return data;
    }
}