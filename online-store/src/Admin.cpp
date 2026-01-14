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
	string check = "SELECT 1 FROM order_items\nWHERE product_id = " + to_string(id) + " LIMIT 1";

	pqxx::result res = db->executeQuery(check);

	if(!res.empty()) {
		cout << "Товар удалить нельзя" << endl;
	} else {
		string query = "DELETE FROM products\nWHERE product_id = " + to_string(id) + ""
				" RETURNING product_id;";

		res = db->executeQuery(query);

		if(!res.empty()) {
			cout << "Продукт с ID: " << id << " удалён";
		} else {
			cout << "Продукт с ID: " << id << " не найден";
		}
	}
}

pqxx::result Admin::viewAllOrders(unique_ptr<DatabaseConnection<string>>& db) {
	return db->executeQuery("SELECT order_id, status FROM orders\nORDER BY order_id;");
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

	cout << "Статус заказа обновлен." << endl;
}

pqxx::result Admin::getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) {
	string query = "SELECT * FROM getOrderStatusHistory(" + to_string(oi) + ");";

	return db->executeQuery(query);
}

pqxx::result Admin::getUserActions(unique_ptr<DatabaseConnection<string>>& db, int ui) {
	string query = "SELECT * FROM getAuditLogByUser(" + to_string(ui) + ");";

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
		file << " \"order_id\":" << res[i]["order_id"].as<int>() << ",\n";
		file << " \"current_status\": \"" << res[i]["current_status"].c_str() << "\",\n";
		file << " \"order_date\": \"" << res[i]["order_date"].c_str() << "\",\n";
		file << " \"number_of_status_changes\": " << res[i]["number_of_status_changes"].as<int>() << ",\n";
		file << " \"last_action\": \"" << res[i]["last_action"].c_str() << "\",\n";
		file << " \"action_date\": \"" << res[i]["action_date"].c_str() << "\",\n";
		file << " \"number_of_audit_actions\": " << res[i]["number_of_audit_actions"].as<int>() << "\n";
		file << "}";
		if (i + 1 < res.size()) {
			file << ",";
		}
		file << "\n";

	}
	file << "]\n";
	file.close();

	cout << "Отчёт успешно создан:" << FileName << endl;
}

void Admin::runAdminMenu(unique_ptr<DatabaseConnection<string>>& db) {
	int choice;

	do {
		cout << "\n\n=== МЕНЮ АДМИНИСТРАТОРА ===" << endl;
		cout << "1. Добавить новый продукт" << endl;
		cout << "2. Вывести список всех продуктов" << endl;
		cout << "3. Обновить информацию о продукте" << endl;
		cout << "4. Удалить продукт" << endl;
		cout << "5. Просмотр всех заказов" << endl;
		cout << "6. Просмотр деталей заказа" << endl;
		cout << "7. Изменить статус заказа" << endl;
		cout << "8. Просмотр истории статусов заказа" << endl;
		cout << "9. Просмотр журнала аудита" << endl;
		cout << "10. Сформировать отчёт" << endl;
		cout << "0. Выход" << endl;
		cout << "Ваш выбор: ";

		cin.clear();
		cin.ignore();

		cin >> choice;

		cout << endl;

		switch (choice) {
			case 1: {
				string name;
				double price;
				int stock_quantity;
				cout << "Название продукта: ";
				cin.ignore();
				getline(cin, name);
				cout << "Цена: ";
				cin >> price;
				cout << "Количество на складе: ";
				cin >> stock_quantity;
				int productId = addProduct(db, name, price, stock_quantity);
				cout << "Продукт добавлен с ID: " << productId << endl;
				break;
			}
			case 2 : {
				pqxx::result res = db->executeQuery("SELECT * FROM products\nORDER BY product_id;");

				cout << "[ID][Навание][Стоимость][Кол-во на складе]" << endl;

				for (auto row : res) {
					cout << row["product_id"].as<int>() << ", "
							<< row["name"].c_str() << ", "
							<< row["price"].as<double>() << ", "
							<< row["stock_quantity"].c_str() << endl;
				}
				break;
			}
			case 3: {
				int id;
				string name;
				double price;
				int stock_quantity;
				cout << "ID продукта для обновления: ";
				cin >> id;
				cin.ignore();
				cout << "Новое название продукта: ";
				getline(cin, name);
				cout << "Новая цена: ";
				cin >> price;
				cout << "Новое количество: ";
				cin >> stock_quantity;
				updateProduct(db, id, name, price, stock_quantity);
				cout << "Продукт обновлен." << endl;
				break;
			}
			case 4: {
				int id;
				cout << "ID продукта для удаления: ";
				cin >> id;
				deleteProduct(db, id);
				break;
			}
			case 5: {
				cout << "[ID][Статус]" << endl;
				auto res = viewAllOrders(db);
				cout << "Все заказы:" << endl;
				for (auto row : res) {
					for (auto field : row) {
						cout << field.c_str() << " ";
					}
					cout << "\n";
				}
				break;
			}
			case 6: {
				int id;
				cout << "ID заказа для просмотра: ";
				cin >> id;
				auto res = viewOrderDetails(db,id);
				cout << "Детали заказ:" << endl;
				cout << "ID пользователя: " << res[0]["user_id"].as<int>() << endl;
				cout << "Статус заказа: " << res[0]["status"].c_str() << endl;
				cout << "Стоимость заказа: " << res[0]["total_price"].as<double>() << endl;
				cout << "Дата последнего изменения статуса:" << res[0]["order_date"].c_str() << endl;
				break;
			}
			case 7: {
				int orderId;
				string status;
				cout << "ID заказа для обновления: ";
				cin >> orderId;
				cin.ignore();
				cout << "Новый статус: ";
				getline(cin, status);
				updateOrderStatus(db, orderId, status);
				break;
			}
			case 8: {
				int orderId;
				cout << "ID заказа для просмотра истории: ";
				cin >> orderId;
				auto res = getOrderStatusHistory(db, orderId);
				cout << "[ID][ID заказа][Старый статус][Новый статус][Дата][ID пользователя]" << endl;
				cout << "История заказа:" << endl;
				for (auto row : res) {
					for (auto field : row) {
						if(string(field.c_str()).empty()) {
							cout << "NULL ";
						} else {
							cout << field.c_str() << " ";
						}
					}
					cout << endl;
				}
				break;
			}
			case 9: {
				int userId;
				cout << "ID пользователя: ";
				cin >> userId;
				auto res = getUserActions(db, userId);
				cout << "Действия пользователя:\n";
				cout << "[ID][Сущность][ID сущности][Операция][ID пользователя][Дата выполнения]" << endl;
				for (auto row : res) {
					for (auto field : row) cout << field.c_str() << " ";
					cout << "\n";
				}
				break;
			}
			case 10: {
				string fileName;
				cout << "Имя CSV файла: ";
				cin.ignore();
				getline(cin, fileName);
				ofstream f(fileName);
				if(!f.is_open()){
					cerr << "Ошибка при создании CSV файла" << endl;
				}
				createCSVReport(db, fileName);
				cout << "CSV отчет создан." << endl;
				f.close();
				break;
			}
			case 0:
				cout << "Выход из меню администратора." << endl;
				return;
			default:
				cout << "Неверный выбор. Попробуйте снова." << endl;
		}
	} while(choice != 0);
}
