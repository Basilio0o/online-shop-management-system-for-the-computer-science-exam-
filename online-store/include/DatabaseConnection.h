#pragma once

#include <pqxx/pqxx>
#include <string>
#include <iostream>

template <typename T>
class DatabaseConnection {
private:
	pqxx::connection conn;
public:
	DatabaseConnection(const T& connectionString);

	pqxx::result executeQuery(const std::string& s);

	void executeNonQuery(const std::string& s);

	pqxx::connection& getConnection();

	bool isConnected();
};
