#pragma once

#include "Singleton.hpp"

#include <hiredis/hiredis.h>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <deque>

// Redis 连接
class RedisConnection {
public:
	RedisConnection(redisContext* context);
	RedisConnection(const RedisConnection&) = delete;
	RedisConnection(RedisConnection&&) = delete;
	RedisConnection& operator=(const RedisConnection&) = delete;
	RedisConnection& operator=(RedisConnection&&) = delete;

	~RedisConnection();

	bool auth(const std::string& password);

	bool set(const std::string& key, const std::string& value);
	bool get(const std::string& key, std::string& value);
	bool del(const char* key);

	// 列表 头插
	bool LPush(const std::string& key, const std::string& value);
	// 列表 头删
	bool LPop(const std::string& key, std::string& value);
	// 列表 尾插
	bool RPush(const std::string& key, const std::string& value);
	// 列表 尾删
	bool RPop(const std::string& key, std::string& value);

	bool HSet(const std::string& key, const std::string& field, const std::string& value);
	bool HGet(const std::string& key, const std::string& field, std::string& value);

	bool exists(const std::string& key);

	bool isVaild() const;

	bool expire(const std::string& key, int seconds);

private:
	redisContext* m_context = nullptr;
};

using RedisConnPtr = std::shared_ptr<RedisConnection>;

// Redis 连接池
class RedisConnPool : public Singleton<RedisConnPool> {
	friend class Singleton<RedisConnPool>;

public:
	static constexpr uint32_t MAX_POOL_SIZE = 100;

public:
	RedisConnPool() = default;
	RedisConnPool(const RedisConnPool&) = delete;
	RedisConnPool(RedisConnPool&&) = delete;
	RedisConnPool& operator=(const RedisConnPool&) = delete;
	RedisConnPool& operator=(RedisConnPool&&) = delete;
	~RedisConnPool();

	void init(const std::string& ip, uint16_t port, uint32_t pool_size);

	bool initialized() const;

	// 获取连接
	RedisConnPtr getConnection(std::chrono::seconds timeout = std::chrono::seconds(3));

	// 归还连接
	void releaseConnection(RedisConnPtr conn);

	// 连接池剩余连接数
	size_t available() const;

	// 连接池大小
	size_t size() const;

private:
	std::deque<RedisConnPtr> m_connections;

	uint32_t m_pool_size = 0;

	mutable std::mutex m_mtx;
	std::condition_variable m_cv;

	// 连接池是否初始化
	std::atomic<bool> m_initialized = false;
};

// Redis 连接 RAII 封装
struct RedisConnGuard {
	explicit RedisConnGuard(RedisConnPtr conn) : m_conn(std::move(conn)) {}

	~RedisConnGuard() {
		if (m_conn) {
			RedisConnPool::getInstance()->releaseConnection(m_conn);
		}
	}

	RedisConnGuard& operator=(RedisConnGuard&&) = default;
	RedisConnGuard(const RedisConnGuard&) = delete;
	RedisConnGuard(RedisConnGuard&&) = default;
	RedisConnGuard& operator=(const RedisConnGuard&) = default;

	RedisConnPtr get() const { return m_conn; }

	RedisConnection* operator->() const { return m_conn.get(); }

private:
	RedisConnPtr m_conn;
};

// RAII redisReply 封装
class RedisReplyGuard {
public:
	explicit RedisReplyGuard(redisReply* reply) : m_reply(reply) {}

	~RedisReplyGuard() {
		if (m_reply != nullptr) {
			freeReplyObject(m_reply);
		}
	}

	RedisReplyGuard(const RedisReplyGuard&) = delete;
	RedisReplyGuard(RedisReplyGuard&&) = default;
	RedisReplyGuard& operator=(const RedisReplyGuard&) = delete;
	RedisReplyGuard& operator=(RedisReplyGuard&&) = default;

	operator bool() const { return m_reply != nullptr; }

	redisReply* operator->() { return m_reply; }

private:
	redisReply* m_reply = nullptr;
};