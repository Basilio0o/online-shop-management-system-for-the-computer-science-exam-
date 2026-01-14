#pragma once

#include <string>
#include "../include/OrderItem.h"
#include "../include/PaymentStrategy.h"
#include "../include/DatabaseConnection.h"
#include <memory>
#include <vector>

using namespace std;

class Order {
private:
	int order_id;
	string status;
	vector<OrderItem> items;
	unique_ptr<PaymentStrategy> payment;
public:
	Order(int id, const string& s);

	void addItem(shared_ptr<Product> p, int q);

	void deleteItem(int pi);

	string getStatus();

	void setStatus(const string& s);

	int getId();

	void setPayment(unique_ptr<PaymentStrategy> p);

	double getTotal_order();
};

class OrderService {
public:
	static vector<shared_ptr<Order>> loadUserOrders(unique_ptr<DatabaseConnection<string>>& db, int user_id);

	static void loadItems(unique_ptr<DatabaseConnection<string>>& db, Order& order);
};
