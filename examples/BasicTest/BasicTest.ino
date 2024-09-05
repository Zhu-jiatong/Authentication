/*
 Name:		BasicTest.ino
 Created:	8/26/2024 10:05:37 PM
 Author:	zhuji
*/

#include "Authentication.h"
#include <SD.h>
#include <iostream>

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);

	try
	{
		if (!SD.begin(SS, SPI, 80000000))
			throw std::runtime_error("Failed to initialize SD card");

		if (SD.exists("/test.db"))
			SD.remove("/test.db");

		SQLite::DbConnection db("/sd/test.db");
		initialiseDatabase(db);

		Authentication auth(db, onUserCreated);
		auth.createUser("admin", "admin");
		auth.createUser("user", "user");

		std::optional<Authentication::UserData> user = auth.authenticate("admin", "admin");
		if (!user)
			throw std::runtime_error("Failed to authenticate admin");

		std::cout << "Authenticated as: \n" << "ID: " << user->id << "\nUsername: " << user->username << std::endl;

		user = auth.authenticate("user", "user");
		if (!user)
			throw std::runtime_error("Failed to authenticate user");

		std::cout << "Authenticated as: \n" << "ID: " << user->id << "\nUsername: " << user->username << std::endl;

		user = auth.authenticate("admin", "user");
		if (user)
			throw std::runtime_error("Authenticated with wrong password");

		user = auth.authenticate("user", "admin");
		if (user)
			throw std::runtime_error("Authenticated with wrong password");

		auth.updateUserUsername(1, "administrator");
		user = auth.authenticate("admin", "admin");
		if (user)
			throw std::runtime_error("Authenticated with old username");

		user = auth.authenticate("administrator", "admin");
		if (!user)
			throw std::runtime_error("Failed to authenticate with new username");

		std::cout << "Authenticated as: \n" << "ID: " << user->id << "\nUsername: " << user->username << std::endl;

		auth.updateUserPassword(1, "password");
		user = auth.authenticate("administrator", "admin");
		if (user)
			throw std::runtime_error("Authenticated with old password");

		user = auth.authenticate("administrator", "password");
		if (!user)
			throw std::runtime_error("Failed to authenticate with new password");

		std::cout << "Authenticated as: \n" << "ID: " << user->id << "\nUsername: " << user->username << std::endl;

		auth.deleteUser(1);
		user = auth.authenticate("administrator", "password");
		if (user)
			throw std::runtime_error("Authenticated after deletion");

		user = auth.authenticate("user", "user");
		if (!user)
			throw std::runtime_error("Failed to authenticate user");

		std::cout << "Authenticated as: \n" << "ID: " << user->id << "\nUsername: " << user->username << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

// the loop function runs over and over again until power down or reset
void loop()
{

}

void initialiseDatabase(SQLite::DbConnection& db)
{
	db.prepare(
		"PRAGMA foreign_keys = ON"
	).evaluate();

	db.prepare(
		"CREATE TABLE IF NOT EXISTS Disks ("
		"ID INTEGER PRIMARY KEY,"
		"Mountpoint TEXT NOT NULL UNIQUE"
		")"
	).evaluate();

	db.prepare(
		"CREATE TABLE IF NOT EXISTS Users ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"Username TEXT NOT NULL UNIQUE,"
		"PasswordHash BLOB NOT NULL"
		")"
	).evaluate();

	db.prepare(
		"CREATE TABLE IF NOT EXISTS FileEntries ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"OwnerID INTEGER NOT NULL,"
		"DiskID INTEGER,"
		"ParentID INTEGER,"
		"Name TEXT,"

		"UNIQUE (ParentID, Name),"
		"FOREIGN KEY (OwnerID) REFERENCES Users(ID) ON DELETE CASCADE,"
		"FOREIGN KEY (DiskID) REFERENCES Disks(ID) ON DELETE CASCADE,"
		"FOREIGN KEY (ParentID) REFERENCES FileEntries(ID) ON DELETE CASCADE"
		")"
	).evaluate();

	db.prepare(
		"CREATE TABLE IF NOT EXISTS UserFilePermissions ("
		"UserID INTEGER NOT NULL,"
		"FileID INTEGER NOT NULL,"
		"Permission TEXT NOT NULL,"

		"PRIMARY KEY (UserID, FileID, Permission),"
		"FOREIGN KEY (UserID) REFERENCES Users(ID) ON DELETE CASCADE,"
		"FOREIGN KEY (FileID) REFERENCES FileEntries(ID) ON DELETE CASCADE"
		")"
	).evaluate();
}

void onUserCreated(std::int64_t id)
{
	std::cout << "User created with ID: " << id << std::endl;
}