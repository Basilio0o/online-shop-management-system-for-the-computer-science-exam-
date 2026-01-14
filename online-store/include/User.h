#pragma once
#include "../include/Order.h"
#include "../include/Product.h"
#include "../include/DatabaseConnection.h"
#include "pqxx/pqxx"
#include <string>
#include <algorithm>
#include <numeric>
#include <vector>

class User {
private:
	int user_id;
	std::string name;
	std::string email;
	std::string role;
	int loyalty_level;
	std::vector<std::shared_ptr<Order>> orders;

	std::string buildOrderItemArray(pqxx::work& txn, std::vector<std::pair<std::shared_ptr<Product>, int>>& list);
public:
	User(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& n, const std::string& e, const std::string& r, int l);

	virtual ~User() = default;

	virtual void updateOrderStatus(std::unique_ptr<DatabaseConnection<std::string>>& db, int id, const std::string& status) = 0;

	bool canChangeStatus(const string& role);

	int createOrder(std::unique_ptr<DatabaseConnection<std::string>>& db, std::vector<std::pair<std::shared_ptr<Product>, int>>& list);

	std::string viewOrderStatus(std::unique_ptr<DatabaseConnection<std::string>>& db, int id);

	void cancelOrder(std::unique_ptr<DatabaseConnection<std::string>>& db, int id);

	std::vector<std::shared_ptr<Order>> orderFilterByStatus(std::unique_ptr<DatabaseConnection<std::string>>& db, const std::string& status);

	double totalAmount(const std::string& status);

	int numbertByStatus(const std::string& status);

	double returnOrder(std::unique_ptr<DatabaseConnection<std::string>>& db, int id);

	int getUserId();

	std::vector<std::shared_ptr<Order>> getOrderArray();

	virtual pqxx::result getOrderStatusHistory(std::unique_ptr<DatabaseConnection<std::string>>& db, int oi) = 0;

	virtual pqxx::result viewOrderDetails(std::unique_ptr<DatabaseConnection<std::string>>& db, int id) = 0;
};
