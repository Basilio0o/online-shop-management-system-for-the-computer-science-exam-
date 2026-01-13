#pragma once
#include "../include/User.h"
#include "../include/DatabaseConnection.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

class Admin : public User {
public:
	Admin(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, int l);

	int addProduct(unique_ptr<DatabaseConnection<string>>& db, const string& product_name, double price, int stock_quantity);

	void updateProduct(unique_ptr<DatabaseConnection<string>>& db, int id, const string& product_name, double price, int stock_quantity);

	void deleteProduct(unique_ptr<DatabaseConnection<string>>& db, int id);

	pqxx::result viewAllOrders(unique_ptr<DatabaseConnection<string>>& db);

	pqxx::result viewOrderDetails(unique_ptr<DatabaseConnection<string>>& db, int id);

	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) override;

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override;

	pqxx::result getUserActions(unique_ptr<DatabaseConnection<string>>& db, int ui);

	void createCSVReport(unique_ptr<DatabaseConnection<string>>& db, const string& FileName);

	void runAdminMenu(unique_ptr<DatabaseConnection<string>>& db) {
		int choice;

		do {
			cout << "=== МЕНЮ АДМИНИСТРАТОРА ===" << endl;
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
					pqxx::result res = db->executeQuery("SELECT * FROM products;");

					cout << "[ID][Навание][Стоимость][Кол-во на складе]";

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
					cout << "Продукт удален." << endl;
					break;
				}
				case 5: {
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
					cout << "Статус заказа обновлен." << endl;
					break;
				}
				case 8: {
					int orderId;
					cout << "ID заказа для просмотра истории: ";
					cin >> orderId;
					auto res = getOrderStatusHistory(db, orderId);
					cout << "История заказа:" << endl;
					for (auto row : res) {
						for (auto field : row) cout << field.c_str() << " ";
						cout << "\n";
					}
					break;
				}
				case 9: {
					int userId;
					cout << "ID пользователя: ";
					cin >> userId;
					auto res = getUserActions(db, userId);
					cout << "Действия пользователя:\n";
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
					break;
				default:
					cout << "Неверный выбор. Попробуйте снова." << endl;
			}
		} while(choice != 0);
	}
};

