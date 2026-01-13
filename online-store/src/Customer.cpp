#include "../include/Customer.h"

void Customer::addToOrder(unique_ptr<DatabaseConnection<string>>& db, int order_id, int product_id, int quantity) {
	auto res = db->executeQuery("SELECT * FROM products\nWHERE product_id = " + to_string(product_id) + ";")[0];

	auto product = make_shared<Product>(res["product_id"].as<int>(), res["name"].c_str(),
			res["price"].as<double>(), res["stock_quantity"].as<int>());

	shared_ptr<Order> order;
	for (auto o : getOrderArray()) {
		if(o->getId() == order_id) {
			order = o;
			break;
		}
	}

	order->addItem(product, quantity);

	string query = "CALL addToOrder(" + to_string(order_id) + ", " + to_string(product_id) + ", " + to_string(quantity) + ");";

	db->executeNonQuery(query);

	cout << "Товар добавлен в заказ";
}

void Customer::removeFromOrder(unique_ptr<DatabaseConnection<string>>& db, int order_id, int product_id) {
	shared_ptr<Order> order;
	for (auto o : getOrderArray()) {
		if(o->getId() == order_id) {
			order = o;
			break;
		}
	}

	order->deleteItem(product_id);

	string query = "CALL deleteFromOrder(" + to_string(order_id) + ", " + to_string(product_id) + ");";

	db->executeNonQuery(query);

	cout << "Товар удалён из заказа";
}

void Customer::makePayment(unique_ptr<DatabaseConnection<string>>& db, PaymentStrategy& p, double amount) {
	p.pay(amount);
}

pqxx::result Customer::getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) {
	string query = "SELECT * FROM order_status_history\nWHERE user_id = "
			+ to_string(getUserId()) + " AND order_id = "
			+ to_string(oi) + ";";

	return db->executeQuery(query);
}

void Customer::updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) {
		cout << "Покупатель не может менять статус заказа";
}
