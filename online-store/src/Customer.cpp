#include "../include/Customer.h"

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

pqxx::result Customer::viewOrderDetails(unique_ptr<DatabaseConnection<string>>& db, int id) {
	return db->executeQuery("SELECT otal_price, status, order_date FROM orders\nWHERE order_id = "
			+ to_string(id) + " AND user_id = " + to_string(getUserId()) + ";");
}

void Customer::runCustomerMenu(unique_ptr<DatabaseConnection<string>>& db) {
    int choice;

    do {
        cout << "\n=== МЕНЮ ПОКУПАТЕЛЯ ===\n";
        cout << "1. Создать новый заказ\n";
        cout << "2. Добавить товар в заказ\n";
        cout << "3. Удалить товар из заказа\n";
        cout << "4. Просмотр моих заказов\n";
        cout << "5. Просмотр статуса заказа\n";
        cout << "6. Оплатить заказ\n";
        cout << "7. Оформить возврат заказа\n";
        cout << "8. Просмотр истории статусов заказа\n";
        cout << "9. Выход\n";
        cout << "Ваш выбор: ";

        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // очищаем буфер

        switch (choice) {
            case 1: {
                vector<pair<shared_ptr<Product>, int>> items;
                int orderId = createOrder(db, items); // создаём пустой заказ
                cout << "Создан новый заказ с ID: " << orderId << endl;
                break;
            }
            case 2: {
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
                cout << "ID заказа: ";
                cin >> orderId;
                cout << "ID продукта для удаления: ";
                cin >> productId;
                cin.ignore();
                removeFromOrder(db, orderId, productId);
                break;
            }
            case 4: {
                auto orders = getOrderArray();
                cout << "Мои заказы:\n";
                for (auto o : orders) {
                    cout << "ID: " << o->getId()
                         << ", Статус: " << o->getStatus()
                         << ", Сумма: " << o->getTotal_order() << "\n";
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
                for (auto o : getOrderArray()) {
                    if (o->getId() == orderId) {
                        amount = o->getTotal_order();
                        break;
                    }
                }


                if (amount > 0.0) {
                    makePayment(db, paymentStrategy, amount);
                    cout << "Заказ оплачен на сумму: " << amount << endl;
                } else {
                    cout << "Заказ не найден или пуст.\n";
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
                cout << "История статусов:\n";
                for (auto row : res) {
                    for (auto field : row) cout << field.c_str() << " ";
                    cout << "\n";
                }
                break;
            }
            case 9:
                cout << "Выход из меню покупателя.\n";
                break;
            default:
                cout << "Неверный выбор. Попробуйте снова.\n";
        }

    } while (choice != 9);
}

