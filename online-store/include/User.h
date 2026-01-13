#pragma once

using namespace std;

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
	string name;
	string email;
	string role;
	int loyalty_level;
	vector<shared_ptr<Order>> orders;

	string buildOrderItemArray(pqxx::work& txn, vector<pair<shared_ptr<Product>, int>>& list);
public:
	User(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, const string& r, int l);

	virtual ~User() = default;

	virtual void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) = 0;

	bool canChangeStatus(const string& role);

	int createOrder(unique_ptr<DatabaseConnection<string>>& db, vector<pair<shared_ptr<Product>, int>>& list);

	string viewOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id);

	void cancelOrder(unique_ptr<DatabaseConnection<string>>& db, int id);

	vector<shared_ptr<Order>> orderFiltherByStatus(const string& status);

	double totalAmount(const string& status);

	int numbertByStatus(const string& status);

	double returnOrder(unique_ptr<DatabaseConnection<string>>& db, int id);

	int getUserId();

	vector<shared_ptr<Order>> getOrderArray();

	virtual pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) = 0;
};
