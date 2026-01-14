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

class Admin : public User {
public:
	Admin(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, int l);

	int addProduct(unique_ptr<DatabaseConnection<string>>& db, const string& product_name, double price, int stock_quantity);

	void updateProduct(unique_ptr<DatabaseConnection<string>>& db, int id, const string& product_name, double price, int stock_quantity);

	void deleteProduct(unique_ptr<DatabaseConnection<string>>& db, int id);

	pqxx::result viewAllOrders(unique_ptr<DatabaseConnection<string>>& db);

	pqxx::result viewOrderDetails(unique_ptr<DatabaseConnection<string>>& db, int id) override;

	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) override;

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override;

	pqxx::result getUserActions(unique_ptr<DatabaseConnection<string>>& db, int ui);

	void createCSVReport(unique_ptr<DatabaseConnection<string>>& db, const string& FileName);

	void runAdminMenu(unique_ptr<DatabaseConnection<string>>& db);
};

