#include "IoContextPool.h"

IoContextPool::~IoContextPool() { stop(); }

void IoContextPool::start(uint32_t pool_size) {
	m_pool_size = pool_size;

	if (m_pool_size <= 0) {
		m_pool_size = 2;
	}
	if (m_pool_size > std::thread::hardware_concurrency()) {
		m_pool_size = std::thread::hardware_concurrency();
	}

	for (int i = 0; i < m_pool_size; ++i) {
		m_contexts.emplace_back(std::make_unique<net::io_context>());
		// work_guard 绑定 io_context
		m_work_guards.emplace_back(net::make_work_guard(*m_contexts[i]));

		m_threads.emplace_back([this, i] {
			fmt::println("IoContextPool::start: thread {} started", i);
			m_contexts[i]->run();
			fmt::println("IoContextPool::start: thread {} stopped", i);
		});
	}
}

void IoContextPool::stop() {
	for (int i = 0; i < m_pool_size; ++i) {
		m_contexts[i]->stop();
		m_work_guards[i].reset();
	}

	for (auto& t : m_threads) {
		t.join();
	}
}

net::io_context& IoContextPool::getIoContext() { return *m_contexts[m_next_index++ % m_pool_size]; }