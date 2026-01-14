#pragma once
#include "../include/User.h"
#include "../include/DatabaseConnection.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

class Admin : public User {
public:
	Admin(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& n, const std::string& e, int l);

	int addProduct(std::unique_ptr<DatabaseConnection<std::string>>& db, const std::string& product_name, double price, int stock_quantity);

	void updateProduct(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const string& product_name, double price, int stock_quantity);

	void deleteProduct(std::unique_ptr<DatabaseConnection<std::string>>& db, int id);

	pqxx::result viewAllOrders(std::unique_ptr<DatabaseConnection<std::string>>& db);

	pqxx::result viewOrderDetails(std::unique_ptr<DatabaseConnection<std::string>>& db, int id) override;

	void updateOrderStatus(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& status) override;

	pqxx::result getOrderStatusHistory(std::unique_ptr<DatabaseConnection<std::string>>& db, int oi) override;

	pqxx::result getUserActions(std::unique_ptr<DatabaseConnection<std::string>>& db, int ui);

	void createCSVReport(std::unique_ptr<DatabaseConnection<std::string>>& db, const std::string& FileName);

	void runAdminMenu(std::unique_ptr<DatabaseConnection<std::string>>& db);
};

