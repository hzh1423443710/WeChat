#pragma once

#include "DAO/user.h"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

// 密码 hash 与验证
struct PasswdHasher {
	static std::string passwd_hash(const std::string& passwd) {
		std::array<uint8_t, SHA256_DIGEST_LENGTH> hash{};
		unsigned int hashLen{};

		//  Message Digest
		EVP_MD_CTX* ctx = EVP_MD_CTX_new();
		EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
		EVP_DigestUpdate(ctx, passwd.data(), passwd.length());
		EVP_DigestFinal_ex(ctx, hash.data(), &hashLen);
		EVP_MD_CTX_free(ctx);

		return to_hex_string(hash);
	}

	static bool passwd_verify(const std::string& passwd, const std::string& hash) {
		return passwd_hash(passwd) == hash;
	}

private:
	static std::string to_hex_string(const std::array<uint8_t, SHA256_DIGEST_LENGTH>& data) {
		std::ostringstream oss;
		for (uint8_t i : data) {
			oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);
		}
		return oss.str();
	}
};

class MysqlUserDao : public UserDao {
public:
	// 根据用户名查询用户
	std::optional<User> queryByUsername(const std::string& username) override;

	// 根据邮箱查询用户
	std::vector<User> queryByEmail(const std::string& email) override;

	// 添加用户
	bool addUser(const User& user) override;

	// 删除用户
	bool deleteUser(int id) override;

	// 更新用户信息
	bool modifyPasswd(const std::string& username, const std::string& newPasswd) override;
	bool modifyAvatar(const std::string& username, const std::string& newAvatar) override;
	bool modifyEmail(const std::string& username, const std::string& newEmail) override;

	static std::string hashPassword(const std::string& password);
	static bool verifyPassword(const std::string& password, const std::string& hash);
};