#include <iostream>
#include <fstream>
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "../include/DatabaseConnection.h"
#include "../include/PaymentStrategy.h"
#include "../include/Product.h"
#include "../include/OrderItem.h"

using namespace std;

class Order {
private:
	int order_id;
	string status;
	vector<OrderItem> items;
	unique_ptr<PaymentStrategy> payment;
public:
	Order(int id, const string& s) : order_id(id), status(s) {}

	void addItem(shared_ptr<Product> p, int q) {
		items.emplace_back(p, q);
	}

	void deleteItem(int pi) {
		items.erase(remove_if(items.begin(), items.end(),
				[pi](OrderItem& item){return item.getProductId() == pi;}), items.end());
	}

	string getStatus() {
		return status;
	}

	void setStatus(const string& s) {
		status = s;
	}

	int getId() {
		return order_id;
	}

	void setPayment(unique_ptr<PaymentStrategy> p) {
		payment = move(p);
	}

	double getTotal_order() {
		double price = 0;
		for(auto item : items) {
			price += item.getTotal_item();
		}
		return price;
	}
};

class OrderRepository {
public:
	static vector<shared_ptr<Order>> loadUserOrders(unique_ptr<DatabaseConnection<string>>& db, int user_id){
		vector<shared_ptr<Order>> orders;

		auto res = db->executeQuery("SELECT * FROM orders WHERE user_id = " + to_string(user_id));

		for (const auto& row : res) {
		auto order = make_shared<Order>(row["order_id"].as<int>(), row["status"].c_str());

		loadItems(db, *order);
		orders.push_back(order);
		}

		return orders;
	}
private:
	static void loadItems(unique_ptr<DatabaseConnection<string>>& db, Order& order) {
		auto res = db->executeQuery("SELECT * FROM order_items\nWHERE order_id = "
				+ to_string(order.getId())
		);

		for (const auto& row : res) {
			auto product = ProductService::load(db, row["product_id"].as<int>());

			order.addItem(product, row["quantity"].as<int>());
		}
	}
};

class User {
private:
	int user_id;
	string name;
	string email;
	string role;
	string password;
	int loyalty_level;
	vector<shared_ptr<Order>> orders;

	string buildOrderItemArray(pqxx::work& txn, vector<pair<shared_ptr<Product>, int>>& list) {
		if (list.empty()) {
			throw runtime_error("Ваш список товаров пуст");
		}
		string result = "ARRAY[";
		for(size_t i = 0; i < list.size(); i++) {
			result += "(" +
					txn.quote(list[i].first->getId()) + "," +
					txn.quote(list[i].second) + ")";
			if (i + 1 < list.size()) {
				result += ", ";
			}

		}
		result += "]::order_items_t[]";
		return result;
	}
public:
	User(unique_ptr<DatabaseConnection<string>>& db, int u, const string& n,
			const string& e, const string& r, const string& p, int l) :
				user_id(u), email(e), role(r), password(p), loyalty_level(l)
	{
		string query = "SET app.current_user_id = " + db->getConnection().quote(to_string(user_id)) + ";";

		db->executeNonQuery(query);

		auto loaded = OrderRepository::loadUserOrders(db, user_id);

		orders = move(loaded);

	}

	virtual ~User() = default;

	virtual void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) = 0;

	bool canChangeStatus(const string& role) {
		auto canChangeStatus = [](const string& role) {
			return role == "admin" || role == "manager";
		};
		return canChangeStatus(role);
	}

	int createOrder(unique_ptr<DatabaseConnection<string>>& db, vector<pair<shared_ptr<Product>, int>>& list) {
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

	string viewOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id) {
		string query = "SELECT getOrderStatus(" + to_string(id) + ");";

		string status = db->executeQuery(query)[0]["getOrderStatus"].c_str();

		return status;
	}

	void cancelOrder(unique_ptr<DatabaseConnection<string>>& db, int id) {

		string query = "CALL update_Order_status(" + to_string(user_id) + ", "
				+ to_string(id) + ", 'canceled')";

		db->executeNonQuery(query);

		cout << "Ваш заказ отменён";
	}

	vector<shared_ptr<Order>> orderFiltherByStatus(const string& status) {
		vector<shared_ptr<Order>> filtheredOrderArray;
		copy_if(orders.begin(), orders.end(), back_inserter(filtheredOrderArray),
				[&status](shared_ptr<Order>& order){return order->getStatus() == status;});

		return filtheredOrderArray;
	}

	double totalAmount(const string& status) {
		double amount = accumulate(orders.begin(), orders.end(), 0.0,
				[&status](double count, shared_ptr<Order>& order)
				{if (order->getStatus() == "completed") return count + order->getTotal_order();});
		return amount;
	}

	int numbertByStatus(const string& status) {
		int number = accumulate(orders.begin(), orders.end(), 0,
				[&status](int count, shared_ptr<Order>& order) {return count + (order->getStatus() == status || status == "ALL");});
		return number;
	}

	double returnOrder(unique_ptr<DatabaseConnection<string>>& db, int id) {
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

	int getUserId() {
		return user_id;
	}

	vector<shared_ptr<Order>> getOrderArray() {
		return orders;
	}

	virtual pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) = 0;
};

class Admin : public User {
public:
	int addProduct(unique_ptr<DatabaseConnection<string>>& db, const string& product_name, double price, int stock_quantity) {
		string query = "INSERT INTO products (name, price,stock_quantity)\n"
				"VALUES (" + db->getConnection().quote(product_name) + ", " + to_string(price) + ", " + to_string(stock_quantity) +
				") RETURNING product_id";

		int id = db->executeQuery(query)[0]["product_id"].as<int>();

		cout << "Товар успешно добавлен (ID = " << id << ")" << endl;

		return id;
	}

	void updateProduct(unique_ptr<DatabaseConnection<string>>& db, int id, const string& product_name, double price, int stock_quantity) {
		string query = "UPDATE products\n"
				"SET name = " + db->getConnection().quote(product_name) +
				", price = " + to_string(price) +
				", stock_quantity = " + to_string(stock_quantity) +
				"\nWHERE product_id = " + to_string(id);

		db->executeNonQuery(query);

		cout << "Информация о товаре с ID = " << id << " обновлена" << endl;
	}

	void deleteProduct(unique_ptr<DatabaseConnection<string>>& db, int id) {
		string query = "DELETE FROM products\nWHERE product_id = " + to_string(id);

		db->executeNonQuery(query);

		cout << "Товар с ID = " << id << " удалён" << endl;
	}

	pqxx::result viewAllOrders(unique_ptr<DatabaseConnection<string>>& db) {
		return db->executeQuery("SELECT order_id, status FROM orders;");
	}

	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) override {
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

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override {
		string query = "CALL getOrderStatusHistory(" + to_string(oi) + ");";

		return db->executeQuery(query);
	}

	pqxx::result getUserActions(unique_ptr<DatabaseConnection<string>>& db, int ui) {
		string query = "CALL getAuditLogByUser(" + to_string(ui) + ");";

		return db->executeQuery(query);
	}

	void createCSVReport(unique_ptr<DatabaseConnection<string>>& db, const string& FileName) {
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
};

class Manager : public User {
public:
	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& s = "completed") override {
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

	void updateStock(unique_ptr<DatabaseConnection<string>>& db, int id, int stock_quantity) {
		string query = "UPDATE products\n"
				"SET stock_quantity = " + to_string(stock_quantity) +
				"\nWHERE product_id = " + to_string(id);

		db->executeNonQuery(query);

		cout << "Информация о товаре с ID = " << id << " обновлена" << endl;
	}

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override {
		string query = "SELECT * FROM order_status_history\nWHERE user_id = "
				+ to_string(getUserId()) + " AND order_id = "
				+ to_string(oi) + ";";

		return db->executeQuery(query);
	}

	pqxx::result getOrderActions(unique_ptr<DatabaseConnection<string>>& db, int ui) {
		string query = "SELECT * FROM audit_log\nWHERE entity_type = 'order';";

		return db->executeQuery(query);
	}
};

class Customer : public User {
public:
	void addToOrder(unique_ptr<DatabaseConnection<string>>& db, int order_id, int product_id, int quantity) {
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

	void removeFromOrder(unique_ptr<DatabaseConnection<string>>& db, int order_id, int product_id) {
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

	void makePayment(unique_ptr<DatabaseConnection<string>>& db, PaymentStrategy& p, double amount) {
		p.pay(amount);
	}

	pqxx::result getOrderStatusHistory(unique_ptr<DatabaseConnection<string>>& db, int oi) override {
		string query = "SELECT * FROM order_status_history\nWHERE user_id = "
				+ to_string(getUserId()) + " AND order_id = "
				+ to_string(oi) + ";";

		return db->executeQuery(query);
	}

	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) override {
			cout << "Покупатель не может менять статус заказа";
	}

};

int main() {

	return 0;
}
