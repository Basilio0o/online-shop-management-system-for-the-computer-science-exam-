#pragma once
#include "../include/User.h"
#include "../include/DatabaseConnection.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

class Manager : public User {
public:
	Manager(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& n, const std::string& e, int l);

	void updateOrderStatus(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& s) override;

	void updateStock(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, int stock_quantity);

	pqxx::result getOrderStatusHistory(std::unique_ptr<DatabaseConnection<std::string>>& db, int oi) override;

	pqxx::result getOrderActions(std::unique_ptr<DatabaseConnection<std::string>>& db, int ui);

	pqxx::result viewOrderDetails(std::unique_ptr<DatabaseConnection<std::string>>& db, int id) override;

	void runManagerMenu(std::unique_ptr<DatabaseConnection<std::string>>& db);
};
