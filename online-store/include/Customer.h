#pragma once
#include "../include/User.h"
#include "../include/DatabaseConnection.h"
#include "../include/OrderItem.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

class Customer : public User {
public:
	void addToOrder(unique_ptr<DatabaseConnection<string>>& db, int order_id, int product_id, int quantity);

	void removeFromOrder(unique_ptr<DatabaseConnection<string>>& db, int order_id, int product_id);

	void makePayment(unique_ptr<DatabaseConnection<string>>& db, PaymentStrategy& p, double amount);

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override;

	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) override;
};
