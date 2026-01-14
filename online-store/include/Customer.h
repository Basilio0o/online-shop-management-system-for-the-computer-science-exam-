#pragma once
#include "../include/User.h"
#include "../include/DatabaseConnection.h"
#include "../include/OrderItem.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "Payment.h"

class Customer : public User {
public:
	Customer(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& n, const string& e, int l);

	void addToOrder(std::unique_ptr<DatabaseConnection<std::string>>& db, int order_id, int product_id, int quantity);

	void removeFromOrder(std::unique_ptr<DatabaseConnection<std::string>>& db, int order_id, int product_id);

	void makePayment(std::unique_ptr<DatabaseConnection<std::string>>& db, Payment& p, double amount);

	pqxx::result getOrderStatusHistory(std::unique_ptr<DatabaseConnection<std::string>>& db, int oi) override;

	void updateOrderStatus(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& status) override;

	pqxx::result viewOrderDetails(std::unique_ptr<DatabaseConnection<std::string>>& db, int id) override;

	void runCustomerMenu(std::unique_ptr<DatabaseConnection<std::string>>& db);

	std::unique_ptr<Payment> choosePaymentMethod();
};
