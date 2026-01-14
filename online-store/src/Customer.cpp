#include "../include/Customer.h"
using namespace std;

Customer::Customer(unique_ptr<DatabaseConnection<string>>& db, int id, const string& n, const string& e, int l) :
	User(db, id, n, e, "customer", l) {}


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

void Customer::makePayment(unique_ptr<DatabaseConnection<string>>& db, Payment& p, double amount) {
	p.pay(amount);
}

pqxx::result Customer::getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) {
	string query = "SELECT old_status, new_status, changed_at FROM order_status_history\nWHERE order_id = "
			+ to_string(oi) + ";";

	return db->executeQuery(query);
}

void Customer::updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) {
		cout << "Покупатель не может менять статус заказа";
}

pqxx::result Customer::viewOrderDetails(unique_ptr<DatabaseConnection<string>>& db, int id) {
	return db->executeQuery("SELECT total_price, status, order_date FROM orders\nWHERE order_id = "
			+ to_string(id) + " AND user_id = " + to_string(getUserId()) + ";");
}

unique_ptr<Payment> Customer::choosePaymentMethod() {
	int ch;
	unique_ptr<Payment> paymentStrategy = nullptr;

	do {
		cout << "\n\n=== МЕНЮ ОПЛАТЫ === " << endl;
		cout << "1. Оплата банковской картой" << endl;
		cout << "2. Оплата электронным кошельком" << endl;
		cout << "3. Оплата через СБП" << endl;
		cout << "0. Отмена" << endl;
		cout << "Ваш выбор: ";
		cin >> ch;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		cout << endl;

		switch (ch) {
			case 1: {
				string cn, ch;
				cout << "Введите номер вашей карты: ";
				cin >> cn;
				cin.ignore();
				cout << "Введите вашу фамилию и имя: ";
				getline(cin, ch);
				paymentStrategy = make_unique<CardPayment>(cn, ch);
				break;
			}
			case 2: {
				string id;
				cout << "ID вашего кошелька: ";
				cin >> id;
				paymentStrategy = make_unique<WalletPayment>(id);
				break;
			}
			case 3: {
				string pn;
				cout << "Введите номер вашего телефона: ";
				cin >> pn;
				paymentStrategy = make_unique<SBPPayment>(pn);
				break;
			}
			case 0:
				cout << "Отмена выбора оплаты." << endl;
				break;
			default:
				cout << "Неверный выбор. Попробуйте снова." << endl;
		}
	} while (ch != 0 && paymentStrategy == nullptr);

	return paymentStrategy;
}

void Customer::runCustomerMenu(unique_ptr<DatabaseConnection<string>>& db) {
    int choice;

    do {
        cout << "\n\n=== МЕНЮ ПОКУПАТЕЛЯ ===" << endl;
        cout << "1. Создать новый заказ" << endl;
        cout << "2. Добавить товар в заказ" << endl;
        cout << "3. Удалить товар из заказа" << endl;
        cout << "4. Просмотр моих заказов" << endl;
        cout << "5. Просмотр статуса заказа" << endl;
        cout << "6. Оплатить заказ" << endl;
        cout << "7. Оформить возврат заказа" << endl;
        cout << "8. Просмотр истории статусов заказа" << endl;;
        cout << "0. Выход" << endl;
        cout << "Ваш выбор: ";

        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << endl;

        switch (choice) {
            case 1: {
				pqxx::result res = db->executeQuery("SELECT * FROM products\nORDER BY product_id;");

				cout << "[ID][Навание][Стоимость][Кол-во на складе]" << endl;

				for (auto row : res) {
					cout << row["product_id"].as<int>() << ", "
							<< row["name"].c_str() << ", "
							<< row["price"].as<double>() << ", "
							<< row["stock_quantity"].as<int>() << endl;
				}

				string name = "name";
				int q;
				vector<pair<shared_ptr<Product>, int>> items;

				cout << "Вводите название товара и его количество (Чтобы закончить, нажмиже Enter на названии)" << endl;
				cout << "Название: ";
				while (getline(cin, name) && !name.empty()) {
					cout << "Количество: ";
					cin >> q;
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
					auto row = find_if(res.begin(), res.end(), [name, q](const auto& r) {
						return r["name"].template as<string>() == name && r["stock_quantity"].template as<int>() >= q;
					});
					shared_ptr<Product> pr = make_shared<Product>(row["product_id"].as<int>(), name,
							row["price"].as<double>(), row["stock_quantity"].as<int>());
					items.emplace_back(pr, q);
					cout << endl;
					break;
					if(row.empty()) {
 							cout << "Проверьте корректность данных" << endl;
					}
					cout << "Название: ";
				}

            	if(items.empty()) {
            		cout << "Ни один товар не добавлен. Заказ не создан" << endl;
            	}
                break;
            }
            case 2: {
            	pqxx::result res = db->executeQuery("SELECT * FROM products\nORDER BY product_id;");

				cout << "[ID][Навание][Стоимость][Кол-во на складе]" << endl;

				for (auto row : res) {
					cout << row["product_id"].as<int>() << ", "
							<< row["name"].c_str() << ", "
							<< row["price"].as<double>() << ", "
							<< row["stock_quantity"].as<int>() << endl;
				}

                int orderId, productId, quantity;
                cout << "ID заказа: ";
                cin >> orderId;
                cout << "ID продукта: ";
                cin >> productId;
                cout << "Количество: ";
                cin >> quantity;
                cin.ignore();
                addToOrder(db, orderId, productId, quantity);
                break;
            }
            case 3: {
                int orderId, productId;
                cout << "ID ваших заказов:" << endl;
                for(auto order : getOrderArray()) {
                	cout << order->getId() << " ";
                }
                cout << "\nID заказа: ";
                cin >> orderId;
                cout << "Содержимое заказа:" << endl;

                auto orders = getOrderArray();
                auto it = std::find_if(orders.begin(), orders.end(), [orderId](const std::shared_ptr<Order>& order) {
                    return order->getId() == orderId;
                });

                if (it != orders.end()) {
                    cout << "[ID товара][Кол-во товара]" << endl;
                    for (auto& item : (*it)->getItems()) {
                        cout << item.getProductId() << ", " << item.getQuantity() << endl;
                    }
                } else {
                    cout << "Заказ с ID " << orderId << " не найден." << endl;
                }

                cout << "\nID продукта для удаления: ";
                cin >> productId;
                cin.ignore();
                removeFromOrder(db, orderId, productId);
                break;
            }
            case 4: {
                auto orders = getOrderArray();
                cout << "Мои заказы:" << endl;
                cout << "[ID][Статус][Сумма]" << endl;
                for (auto o : orders) {
                    cout << o->getId() << ", " << o->getStatus() << ", " << o->getTotal_order() << endl;
                }
                break;
            }
            case 5: {
                int orderId;
                cout << "ID заказа: ";
                cin >> orderId;
                cin.ignore();
                string status = viewOrderStatus(db, orderId);
                cout << "Статус заказа: " << status << endl;
                break;
            }
            case 6: {
                int orderId;
                cout << "ID заказа для оплаты: ";
                cin >> orderId;
                cin.ignore();


                double amount = 0.0;
                auto orders = getOrderArray();
				auto it = std::find_if(orders.begin(), orders.end(), [orderId](const std::shared_ptr<Order>& order) {
				  return order->getId() == orderId;
				});

				if (it != orders.end()) {
					amount = (*it)->getTotal_order();
				} else {
					cout << "Заказ с ID: " << orderId << " не найден" << endl;
					break;
				}

                if (amount > 0.0) {
                	auto payment = choosePaymentMethod();
                    makePayment(db, *payment, amount);
                    cout << "Заказ оплачен на сумму: " << amount << endl;
                } else {
                    cout << "Заказ не найден или пуст." << endl;
                }
                break;
            }
            case 7: {
                int orderId;
                cout << "ID заказа для возврата: ";
                cin >> orderId;
                cin.ignore();
                try {
                    double refund = returnOrder(db, orderId);
                    cout << "Возврат выполнен. Сумма возврата: " << refund << endl;
                } catch (const runtime_error& e) {
                    cout << e.what() << endl;
                }
                break;
            }
            case 8: {
                int orderId;
                cout << "ID заказа для просмотра истории статусов: ";
                cin >> orderId;
                cin.ignore();
                auto res = getOrderStatusHistory(db, orderId);
                cout << "История статусов:" << endl;
                cout << "[Старый статус][Новый статус][Дата]" << endl;
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
            case 0:
                cout << "Выход из меню покупателя." << endl;
                break;
            default:
                cout << "Неверный выбор. Попробуйте снова." << endl;
        }
    } while (choice != 0);
}



