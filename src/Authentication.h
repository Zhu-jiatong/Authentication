/*
 Name:		Authentication.h
 Created:	8/26/2024 10:04:54 PM
 Author:	zhuji
 Editor:	http://www.visualmicro.com
*/

#ifndef _Authentication_h
#define _Authentication_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "SQLitePlusPlus.h"
#include <string>
#include <array>
#include <optional>

class Authentication
{
public:
	using SHA256 = std::array<uint8_t, 32>;
	struct UserData
	{
		std::int64_t id;
		std::string username;
		SHA256 passwordHash;
	};

	Authentication(SQLite::DbConnection& db) :db(db) {}

	UserData createUser(const std::string& username, const std::string& password);

	std::optional<UserData> authenticate(const std::string& username, const std::string& password);
	std::optional<UserData> getUserData(std::int64_t id);

	void updateUserUsername(std::int64_t id, const std::string& newUsername);
	void updateUserPassword(std::int64_t id, const std::string& newPassword);

	void deleteUser(std::int64_t id);

	static SHA256 hashPassword(const std::string& password);

private:
	SQLite::DbConnection& db;
};

#endif

