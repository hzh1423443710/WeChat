#include "Listener.h"
#include "HttpConnection.h"
#include "pool/IoContextPool.h"

Listener::Listener(net::io_context& ioc, const tcp::endpoint& endpoint)
	: m_ioc(ioc), m_acceptor(ioc, endpoint) {}

void Listener::start() {
	m_acceptor.async_accept(
		[self = shared_from_this()](const std::error_code& ec, tcp::socket socket) {
			if (ec) {
				fmt::println("accept:{}", ec.message());
				return;
			}

			// auto ep = socket.remote_endpoint();
			// fmt::println("{}:{} connected", ep.address().to_string(), ep.port());

			self->on_accept(std::move(socket));
		});
}

void Listener::on_accept(tcp::socket socket) {
	// start a http connection
	auto& ioc = IoContextPool::getInstance()->getIoContext();
	std::make_shared<HttpConnection>(ioc, std::move(socket))->start();

	// continue to accept
	start();
}
