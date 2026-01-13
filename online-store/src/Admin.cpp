#include "../include/Admin.h"


Admin::Admin(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, int l) :
	User(db, id, n, e, "admin", l) {}

int Admin::addProduct(unique_ptr<DatabaseConnection<string>>& db, const string& product_name, double price, int stock_quantity) {
	string query = "INSERT INTO products (name, price,stock_quantity)\n"
			"VALUES (" + db->getConnection().quote(product_name) + ", " + to_string(price) + ", " + to_string(stock_quantity) +
			") RETURNING product_id";

	int id = db->executeQuery(query)[0]["product_id"].as<int>();

	cout << "Товар успешно добавлен (ID = " << id << ")" << endl;

	return id;
}

void Admin::updateProduct(unique_ptr<DatabaseConnection<string>>& db, int id, const string& product_name, double price, int stock_quantity) {
	string query = "UPDATE products\n"
			"SET name = " + db->getConnection().quote(product_name) +
			", price = " + to_string(price) +
			", stock_quantity = " + to_string(stock_quantity) +
			"\nWHERE product_id = " + to_string(id);

	db->executeNonQuery(query);

	cout << "Информация о товаре с ID = " << id << " обновлена" << endl;
}

void Admin::deleteProduct(unique_ptr<DatabaseConnection<string>>& db, int id) {
	string query = "DELETE FROM products\nWHERE product_id = " + to_string(id);

	db->executeNonQuery(query);

	cout << "Товар с ID = " << id << " удалён" << endl;
}

pqxx::result Admin::viewAllOrders(unique_ptr<DatabaseConnection<string>>& db) {
	return db->executeQuery("SELECT order_id, status FROM orders;");
}
pqxx::result Admin::viewOrderDetails(unique_ptr<DatabaseConnection<string>>& db, int id) {
	return db->executeQuery("SELECT user_id, status, total_price, order_date FROM orders\nWHERE order_id = "
			+ to_string(id) + ";");
}

void Admin::updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) {
	string query = "CALL update_Order_status(" + to_string(getUserId()) + ", "
			+ to_string(id) + ", " + db->getConnection().quote(status) + ");";

	for(auto order : getOrderArray()) {
		if (order->getId() == id) {
			order->setStatus(status);
		}
	}

	db->executeNonQuery(query);

	cout << "Статус заказа обновлён";
}

pqxx::result Admin::getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) {
	string query = "CALL getOrderStatusHistory(" + to_string(oi) + ");";

	return db->executeQuery(query);
}

pqxx::result Admin::getUserActions(unique_ptr<DatabaseConnection<string>>& db, int ui) {
	string query = "CALL getAuditLogByUser(" + to_string(ui) + ");";

	return db->executeQuery(query);
}

void Admin::createCSVReport(unique_ptr<DatabaseConnection<string>>& db, const string& FileName) {
	ofstream file(FileName);
	if(!file.is_open()) {
		cerr << "Ошибка! Не удалось открыть файл для чтения" << endl;
	}

	pqxx::result res = db->executeQuery("SELECT * FROM createOrderAuditHistory();");

	file << "[\n";

	for (size_t i = 0; i < res.size(); i++) {
		file << "{\n";
		file << " \"order_id\":" << res[0]["order_id"].as<int>() << ",\n";
		file << " \"current_status\": \"" << res[0]["current_status"].c_str() << "\",\n";
		file << " \"order_date\": \"" << res[0]["order_date"].c_str() << "\",\n";
		file << " \"number_of_status_changes\": \"" << res[0]["number_of_status_changes"].as<int>() << "\",\n";
		file << " \"last_action\": \"" << res[0]["last_action"].c_str() << "\",\n";
		file << " \"action_date\": \"" << res[0]["action_date"].c_str() << "\",\n";
		file << " \"number_of_audit_actions\": " << res[0]["number_of_audit_actions"].as<int>() << ",\n";
		if (i + 1 < res.size()) {
			file << ",";
		}
		file << "}";
	}
	file << "]\n";
	file.close();

	cout << "Отчёт успешно создан:" << FileName << endl;
}
