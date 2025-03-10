#pragma once

#include "net.h"

class Listener : public std::enable_shared_from_this<Listener> {
public:
	Listener(net::io_context& ioc, const tcp::endpoint& endpoint);

	void start();

private:
	void on_accept(tcp::socket socket);

private:
	net::io_context& m_ioc;
	tcp::acceptor m_acceptor;
};