#include "../include/User.h"


string User::buildOrderItemArray(pqxx::work& txn, vector<pair<shared_ptr<Product>, int>>& list) {
	if (list.empty()) {
		throw runtime_error("Ваш список товаров пуст");
	}
	string result = "ARRAY[";
	for(size_t i = 0; i < list.size(); i++) {
		result += "(" +
				txn.quote(list[i].first->getProductId()) + "," +
				txn.quote(list[i].second) + ")";
		if (i + 1 < list.size()) {
			result += ", ";
		}

	}
	result += "]::order_items_t[]";
	return result;
}

User::User(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, const string& r, int l) :
	user_id(id), name(n), email(e), role(r), loyalty_level(l) {
	string query = "SET app.current_user_id = " + db->getConnection().quote(to_string(user_id)) + ";";

	db->executeNonQuery(query);

	auto loaded = OrderService::loadUserOrders(db, user_id);

	orders = move(loaded);
}

bool User::canChangeStatus(const string& role) {
	auto canChangeStatus = [](const string& role) {
		return role == "admin" || role == "manager";
	};
	return canChangeStatus(role);
}

int User::createOrder(unique_ptr<DatabaseConnection<string>>& db, vector<pair<shared_ptr<Product>, int>>& list) {
	pqxx::work txn(db->getConnection());

	string itemsArray = buildOrderItemArray(txn, list);

	string query = "CALL createOrder(" +
			txn.quote(user_id) + ", " +
			itemsArray + ")";
	txn.exec(query);

	pqxx::result res = txn.exec("SELECT order_id\n"
			"FROM orders\n"
			"ORDER BY order_date DESC\n"
			"LIMIT 1;");

	auto order = make_shared<Order>(res[0]["order_id"].as<int>(), "pending");

	for(auto item : list) {
		order->addItem(item.first, item.second);
	}

	orders.emplace_back(order);

	cout << "Заказ успешно создан (ID = " << res[0]["order_id"].as<int>() << ")";

	txn.commit();

	return res[0]["order_id"].as<int>();

}

string User::viewOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id) {
	string query = "SELECT getOrderStatus(" + to_string(id) + ");";

	string status = db->executeQuery(query)[0]["getOrderStatus"].c_str();

	return status;
}

void User::cancelOrder(unique_ptr<DatabaseConnection<string>>& db, int id) {

	string query = "CALL update_Order_status(" + to_string(user_id) + ", "
			+ to_string(id) + ", 'canceled')";

	db->executeNonQuery(query);

	cout << "Ваш заказ отменён";
}

vector<shared_ptr<Order>> User::orderFiltherByStatus(const string& status) {
	vector<shared_ptr<Order>> filtheredOrderArray;
	copy_if(orders.begin(), orders.end(), back_inserter(filtheredOrderArray),
			[&status](shared_ptr<Order>& order){return order->getStatus() == status;});

	return filtheredOrderArray;
}

double User::totalAmount(const string& status) {
	double amount = accumulate(orders.begin(), orders.end(), 0.0,
			[&status](double count, shared_ptr<Order>& order)
			{if (order->getStatus() == "completed") return count + order->getTotal_order();});
	return amount;
}

int User::numbertByStatus(const string& status) {
	int number = accumulate(orders.begin(), orders.end(), 0,
			[&status](int count, shared_ptr<Order>& order) {return count + (order->getStatus() == status || status == "ALL");});
	return number;
}

double User::returnOrder(unique_ptr<DatabaseConnection<string>>& db, int id) {
	string query = "CALL canReturnOrder(" + to_string(id) + ")";

	pqxx::result res = db->executeQuery(query);

	bool f = res[0]["canReturnOrder"].as<bool>();

	if(!f) {
		throw runtime_error("Возврат невозможе");
	}
	query = "CALL update_Order_status(" + to_string(getUserId()) + ", " + to_string(id) + ", 'returned')\nRETURNING total_price";

	res = db->executeQuery(query);

	return res[0]["total_price"].as<double>();
}

int User::getUserId() {
	return user_id;
}

vector<shared_ptr<Order>> User::getOrderArray() {
	return orders;
}
