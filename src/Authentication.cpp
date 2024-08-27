/*
 Name:		Authentication.cpp
 Created:	8/26/2024 10:04:54 PM
 Author:	zhuji
 Editor:	http://www.visualmicro.com
*/

#include "Authentication.h"
#include <mbedtls/sha256.h>

std::optional<Authentication::UserData> Authentication::authenticate(const std::string& username, const std::string& password)
{
	SHA256 hash = hashPassword(password);
	SQLite::SQLStatement stmt = db.prepare("SELECT ID FROM Users WHERE username = ? AND passwordHash = ?");
	stmt.bind(username, std::vector<uint8_t>(hash.begin(), hash.end()));
	if (!stmt.evaluate())
		return std::nullopt;

	std::int64_t id = stmt.getColumnValue<std::int64_t>(0);
	return UserData{ id, username, hash };
}

Authentication::UserData Authentication::createUser(const std::string& username, const std::string& password)
{
	SHA256 hash = hashPassword(password);
	SQLite::SQLStatement stmt = db.prepare("INSERT INTO Users (Username, PasswordHash) VALUES (?, ?) RETURNING ID");
	stmt.bind(username, std::vector<uint8_t>(hash.begin(), hash.end()));
	if (!stmt.evaluate())
		throw std::runtime_error("Failed to create user");

	std::int64_t id = stmt.getColumnValue<std::int64_t>(0);
	return { id, username, hash };
}

std::optional<Authentication::UserData> Authentication::getUserData(std::int64_t id)
{
	SQLite::SQLStatement stmt = db.prepare("SELECT Username, PasswordHash FROM Users WHERE ID = ?");
	stmt.bind(id);
	if (!stmt.evaluate())
		return std::nullopt;

	std::string username = stmt.getColumnValue<std::string>(0);
	std::vector<uint8_t> hash = stmt.getColumnValue<std::vector<uint8_t>>(1);
	SHA256 passwordHash;
	std::move(hash.begin(), hash.end(), passwordHash.begin()); // possible optimisation: parallel move

	return UserData{ id, username, passwordHash };
}

void Authentication::updateUserUsername(std::int64_t id, const std::string& newUsername)
{
	db.prepare("UPDATE Users SET Username = ? WHERE ID = ?")
		.bind(newUsername, id)
		.evaluate();
}

void Authentication::updateUserPassword(std::int64_t id, const std::string& newPassword)
{
	SHA256 hash = hashPassword(newPassword);
	db.prepare("UPDATE Users SET PasswordHash = ? WHERE ID = ?")
		.bind(std::vector<uint8_t>(hash.begin(), hash.end()), id)
		.evaluate();
}

void Authentication::deleteUser(std::int64_t id)
{
	db.prepare("DELETE FROM Users WHERE ID = ?")
		.bind(id)
		.evaluate();
}

Authentication::SHA256 Authentication::hashPassword(const std::string& password)
{
	SHA256 hash;
	int rc = mbedtls_sha256(reinterpret_cast<const uint8_t*>(password.c_str()), password.size(), hash.data(), 0);
	if (rc != 0)
		throw std::runtime_error("Failed to hash password");

	return hash;
}
