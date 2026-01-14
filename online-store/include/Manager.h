#pragma once
#include "../include/User.h"
#include "../include/DatabaseConnection.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

class Manager : public User {
public:
	Manager(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, int l);

	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& s) override;

	void updateStock(unique_ptr<DatabaseConnection<string>>& db, int id, int stock_quantity);

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override;

	pqxx::result getOrderActions(unique_ptr<DatabaseConnection<string>>& db, int ui);

	pqxx::result viewOrderDetails(unique_ptr<DatabaseConnection<string>>& db, int id) override;

	void runManagerMenu(unique_ptr<DatabaseConnection<string>>& db);
};
