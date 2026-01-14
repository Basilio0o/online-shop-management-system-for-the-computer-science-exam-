#pragma once
#include <algorithm>
#include <string>
#include "../include/OrderItem.h"
#include "../include/Payment.h"
#include "../include/DatabaseConnection.h"
#include <memory>
#include <vector>

class Order {
private:
	int order_id;
	std::string status;
	std::vector<OrderItem> items;
	std::unique_ptr<Payment> payment;
public:
	Order(int id, const std::string& s);

	void addItem(std::shared_ptr<Product> p, int q);

	void deleteItem(int pi);

	string getStatus();

	void setStatus(const std::string& s);

	int getId();

	vector<OrderItem> getItems();

	void setPayment(std::unique_ptr<Payment> p);

	double getTotal_order();
};

class OrderService {
public:
	static std::vector<std::shared_ptr<Order>> loadUserOrders(std::unique_ptr<DatabaseConnection<std::string>>& db, int user_id);

	static void loadItems(std::unique_ptr<DatabaseConnection<std::string>>& db, Order& order);
};
