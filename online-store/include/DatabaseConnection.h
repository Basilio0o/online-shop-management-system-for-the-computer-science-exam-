#pragma once

#include <pqxx/pqxx>
#include <string>
using namespace std;

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

template <typename T>
DatabaseConnection<T>::DatabaseConnection(const T& connectionString) : conn(connectionString) {}

template <typename T>
pqxx::result DatabaseConnection<T>::executeQuery(const string& s) {
	pqxx::result res;

	pqxx::work txn(conn);

	res = txn.exec(s);

	txn.commit();

	return res;
}

template <typename T>
void DatabaseConnection<T>::executeNonQuery(const string& s) {
	pqxx::work txn(conn);

	txn.exec(s);

	txn.commit();
}

template <typename T>
pqxx::connection& DatabaseConnection<T>::getConnection() {
	return conn;
}

template <typename T>
bool DatabaseConnection<T>::isConnected() {
	return conn.is_open();
}
