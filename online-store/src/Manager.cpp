#include "../include/Manager.h"
#include "../include/Order.h"
using namespace std;

Manager::Manager(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, int l) :
	User(db, id, n, e, "manager", l) {}

void Manager::updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& s) {
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
	string query = "SELECT * FROM order_status_history\nWHERE changed_by = "
			+ to_string(getUserId()) + " AND order_id = "
			+ to_string(oi) + ";";

	return db->executeQuery(query);
}

pqxx::result Manager::viewOrderDetails(unique_ptr<DatabaseConnection<string>>& db, int id) {
	return db->executeQuery("SELECT user_id, total_price, order_date FROM orders\nWHERE order_id = "
			+ to_string(id) + " AND status = 'pending';");
}

pqxx::result Manager::getOrderActions(unique_ptr<DatabaseConnection<string>>& db, int ui) {
	string query = "SELECT * FROM audit_log\nWHERE entity_type = 'order';";

	return db->executeQuery(query);
}

void Manager::runManagerMenu(unique_ptr<DatabaseConnection<string>>& db) {
	int choice;
	do {
		cout << "\n\n=== МЕНЮ МЕНЕДЖЕРА ===" << endl;
		cout << "1. Просмотр заказов в ожидании утверждения" << endl;
		cout << "2. Утвердить заказ" << endl;
		cout << "3. Обновить количество товара на складе" << endl;
		cout << "4. Просмотр деталей заказа" << endl;
		cout << "5. Просмотр истории утверждённых заказов" << endl;
		cout << "6. Просмотр истории статусов заказов" << endl;
		cout << "0. Выход" << endl;
		cout << "Ваш выбор: ";

		cin >> choice;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		cout << endl;

		switch(choice) {
			case 1: {
				auto orders = orderFilterByStatus(db, "pending");
				cout << "Заказы в ожидании утверждения:" << endl;
				cout << "[ID][Статус][Сумма]" << endl;
				for (auto o : orders) {
					cout << o->getId() << ", "
							<< o->getStatus() << ", "
							<< o->getTotal_order() << endl;
				}
				break;
			}
			case 2: {
				int orderId;
				cout << "Введите ID заказа для утверждения: ";
				cin >> orderId;
				cin.ignore();
				updateOrderStatus(db, orderId, "completed");
				break;
			}
			case 3: {
				pqxx::result res = db->executeQuery("SELECT * FROM products\nORDER BY product_id;");

				cout << "[ID][Навание][Стоимость][Кол-во на складе]" << endl;

				for (auto row : res) {
					cout << row["product_id"].as<int>() << ", "
							<< row["name"].c_str() << ", "
							<< row["price"].as<double>() << ", "
							<< row["stock_quantity"].c_str() << endl;
				}

				cout << endl;

				int productId, stock_quantity;
				cout << "Введите ID продукта: ";
				cin >> productId;
				cout << "Введите новое количество на складе: ";
				cin >> stock_quantity;
				cin.ignore();
				updateStock(db, productId, stock_quantity);
				break;
			}
			case 4: {
				int orderId;
				cout << "Введите ID заказа для просмотра деталей: ";
				cout << "[ID][Стоимость][Дата]" << endl;
				cin >> orderId;
				cin.ignore();
				auto res = viewOrderDetails(db, orderId);
				cout << "Детали заказа:\n";
				for (auto row : res) {
					for (auto field : row) cout << field.c_str() << " ";
					cout << endl;
				}
				break;
			}
			case 5: {
				auto orders = orderFilterByStatus(db, "completed");
				cout << "История утверждённых заказов:" << endl;
				cout << "[ID][Статус][Сумма]" << endl;
				for (auto o : orders) {
					cout << o->getId() << ", " << o->getStatus()
						 << ", " << o->getTotal_order() << endl;
				}
				break;
			}
			case 6: {
				int orderId;
				cout << "Введите ID заказа для просмотра истории статусов: ";
				cin >> orderId;
				cin.ignore();
				auto res = getOrderStatusHistory(db, orderId);
				cout << "История статусов заказа:" << endl;
				cout << "[ID][ID заказа][Старый статус][Новый статус][Дата][ID пользователя]" << endl;
				for (auto row : res) {
					for (auto field : row) cout << field.c_str() << " ";
					cout << endl;
				}
				break;
			}
			case 0:
				cout << "Выход из меню менеджера." << endl;
				break;
			default:
				cout << "Неверный выбор. Попробуйте снова." << endl;
		}

	} while (choice != 0);
}
