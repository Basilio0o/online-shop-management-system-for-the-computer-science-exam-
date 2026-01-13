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
	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& s = "completed") override;
	void updateStock(unique_ptr<DatabaseConnection<string>>& db, int id, int stock_quantity);

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override;

	pqxx::result getOrderActions(unique_ptr<DatabaseConnection<string>>& db, int ui);
};
