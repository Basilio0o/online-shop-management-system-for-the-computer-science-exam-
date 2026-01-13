#include "../include/Manager.h"
#include "../include/Order.h"

void Manager::updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& s = "completed") {
	string query = "CALL update_Order_status(" + to_string(getUserId()) + ", "
			+ to_string(id) + ", "
			+ db->getConnection().quote(s) + ");";

	db->executeNonQuery(query);

	for(auto order : getOrderArray()) {
		if (order->getId() == id) {
			order->setStatus("completed");
		}
	}

	cout << "Заказ утверждён";
}

void Manager::updateStock(unique_ptr<DatabaseConnection<string>>& db, int id, int stock_quantity) {
	string query = "UPDATE products\n"
			"SET stock_quantity = " + to_string(stock_quantity) +
			"\nWHERE product_id = " + to_string(id);

	db->executeNonQuery(query);

	cout << "Информация о товаре с ID = " << id << " обновлена" << endl;
}

pqxx::result Manager::getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi){
	string query = "SELECT * FROM order_status_history\nWHERE user_id = "
			+ to_string(getUserId()) + " AND order_id = "
			+ to_string(oi) + ";";

	return db->executeQuery(query);
}

pqxx::result Manager::getOrderActions(unique_ptr<DatabaseConnection<string>>& db, int ui) {
	string query = "SELECT * FROM audit_log\nWHERE entity_type = 'order';";

	return db->executeQuery(query);
}
