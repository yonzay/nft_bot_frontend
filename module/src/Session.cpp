#include "Session.h"

const char* Session::host = "license-service-1418865070.us-east-1.elb.amazonaws.com";
const char* Session::port = "80";

void Session::new_session() {
	net::io_context ioc;
	std::make_shared<Session>(ioc)->run(Session::host, Session::port, "/api/v4/user/session");
	ioc.run();
}