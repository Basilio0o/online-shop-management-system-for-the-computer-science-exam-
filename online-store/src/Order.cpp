#include "../include/Order.h"
#include "../include/DatabaseConnection.h"
using namespace std;

Order::Order(int id, const string& s) : order_id(id), status(s) {}

void Order::addItem(shared_ptr<Product> p, int q) {
	items.emplace_back(p, q);
}

void Order::deleteItem(int pi) {
	items.erase(remove_if(items.begin(), items.end(),
			[pi](OrderItem& item){return item.getProductId() == pi;}), items.end());
}

string Order::getStatus() {
	return Order::status;
}

void Order::setStatus(const string& s) {
	status = s;
}

int Order::getId() {
	return order_id;
}

void Order::setPayment(unique_ptr<Payment> p) {
	payment = move(p);
}

double Order::getTotal_order() {
	double price = 0;
	for(auto item : items) {
		price += item.getTotal_item();
	}
	return price;
}

void OrderService::loadItems(unique_ptr<DatabaseConnection<string>>& db, Order& order) {
	auto res = db->executeQuery("SELECT * FROM order_items\nWHERE order_id = "
			+ to_string(order.getId())
	);

	for (const auto& row : res) {
		auto product = ProductService::load(db, row["product_id"].as<int>());

		order.addItem(product, row["quantity"].as<int>());
	}
}

vector<OrderItem> Order::getItems(){
	return items;
}

vector<shared_ptr<Order>> OrderService::loadUserOrders(unique_ptr<DatabaseConnection<string>>& db, int user_id){
	vector<shared_ptr<Order>> orders;

	auto res = db->executeQuery("SELECT * FROM orders WHERE user_id = " + to_string(user_id));

	for (const auto& row : res) {
	auto order = make_shared<Order>(row["order_id"].as<int>(), row["status"].c_str());

	loadItems(db, *order);
	orders.push_back(order);
	}

	return orders;
}

