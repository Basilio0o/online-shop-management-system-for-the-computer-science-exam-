#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

template <typename T>
class DatabaseConnection {
private:
	pqxx::connection conn;

public:
	DatabaseConnection(const T& connectionString) : conn(connectionString) {}

	pqxx::result executeQuery(const string& s) {
		pqxx::result res;

		pqxx::work txn(conn);

		res = txn.exec(s);

		txn.commit();

		return res;
	}

	void executeNonQuery(const string& s) {
		pqxx::work txn(conn);

		txn.exec(s);

		txn.commit();
	}

	pqxx::connection& getConnection() {
		return conn;
	}

	bool isConnected() {
		return conn.is_open();
	}

};

class PaymentStrategy {
public:
	virtual void pay(double amount) = 0;
	virtual ~PaymentStrategy() = default;
};

class CardPayment : public PaymentStrategy {
private:
	string cardNumber;
	string cardHolder;
public:
	CardPayment(const string& cn, const string& ch) : cardNumber(cn), cardHolder(ch) {}

	void pay(double amount) override {
		cout << "Оплата картой\nСумма: " << amount << "руб.\nКарта: **** **** **** "
				<< cardNumber.substr(cardNumber.size() - 4) << "\nСовершил оплату: " << cardHolder << endl;
	}
};

class WalletPayment : public PaymentStrategy {
private:
	string walletId;
public:
	WalletPayment(const string& id) : walletId(id) {}

	void pay(double amount) override {
		cout << "Оплата электронным кошельком\nСумма: " << amount << "руб.\nКошелёк: " << walletId << endl;
	}
};

class SBPPayment : public PaymentStrategy {
private:
	string phoneNumber;
public:
	SBPPayment(const string& pn) : phoneNumber(pn) {}

	void pay(double amount) override {
		cout << "Оплата Через СПБ\nСумма: " << amount << "руб.\nТелефон: " << phoneNumber << endl;
	}
};

class Product {
private:
	int product_id;
	string name;
	double price;
	int stock_quantity;
public:
	Product(int id, const string& n, double p, int sq) : product_id(id), name(n), price(p), stock_quantity(sq) {}

	int getId() const {
		return product_id;
	}

	string getName() const {
		return name;
	}

	double getPrice() const {
		return price;
	}

	int getStock() const {
		return stock_quantity;
	}

	void decreaseStock(int q) {
		if(q > stock_quantity) {
			throw runtime_error("Недостаточно товара на склае");
		}
		stock_quantity -= q;
	}
};

class ProductRepository {
public:
	static shared_ptr<Product> load(unique_ptr<DatabaseConnection<string>>& db, int product_id) {
	auto res = db->executeQuery(
	  "SELECT * FROM products WHERE product_id = " + to_string(product_id)
	);

	if (res.empty())
	  throw runtime_error("Товар не найден");

	return make_shared<Product>(
	  res[0]["product_id"].as<int>(),
	  res[0]["name"].c_str(),
	  res[0]["price"].as<double>(),
	  res[0]["stock_quantity"].as<int>()
	);
	}
};

class OrderItem {
private:
	shared_ptr<Product> product;
	int quantity;
public:
	OrderItem(shared_ptr<Product>& p, int q) : product(p), quantity(q) {}

	double getTotal() {
		return quantity*product->getPrice();
	}

	int getProductId() {
		return product->getId();
	}

	int getQuantity() {
		return quantity;
	}

};

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
};

class OrderRepository {
public:
	static vector<shared_ptr<Order>> loadUserOrders(unique_ptr<DatabaseConnection<string>>& db, int user_id) {
    vector<shared_ptr<Order>> orders;

    auto res = db->executeQuery(
      "SELECT * FROM orders WHERE user_id = " + to_string(user_id)
    );

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
			auto product = ProductRepository::load(db, row["product_id"].as<int>());

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

	int totalAmountByStatus(const string& status) {
		int amount = accumulate(orders.begin(), orders.end(), 0,
				[&status](int count, shared_ptr<Order>& order) {return count + (order->getStatus() == status || status == "ALL");});
		return amount;
	}

	bool canReturn(unique_ptr<DatabaseConnection<string>>& db, int id) {
		string query = "CALL canReturnOrder(" + to_string(id) + ")";

		pqxx::result res = db->executeQuery(query);

		return res[0]["canReturnOrder"].as<bool>();
	}

	int getUserId() {
		return user_id;
	}

	vector<shared_ptr<Order>> getOrderArray() {
		return orders;
	}
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

		db->executeNonQuery(query);

		cout << "Статус заказа обновлён";
	}
};

class Manager : public User {
public:
	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& s = "completed") override {
		string query = "CALL update_Order_status(" + to_string(getUserId()) + ", "
				+ to_string(id) + db->getConnection().quote(s) + ");";

		db->executeNonQuery(query);

		cout << "Заказ утверждён";
	}

	void updateStock(unique_ptr<DatabaseConnection<string>>& db, int id, int stock_quantity) {
		string query = "UPDATE products\n"
				"SET stock_quantity = " + to_string(stock_quantity) +
				"\nWHERE product_id = " + to_string(id);

		db->executeNonQuery(query);

		cout << "Информация о товаре с ID = " << id << " обновлена" << endl;
	}
};

class Customer : public User {
public:
	void addToOrder(unique_ptr<DatabaseConnection<string>>& db, int order_id, int product_id, int quantity) {
		auto res = db->executeQuery("SELECT * FROM product\nWHERE product_id = " + to_string(product_id) + ";")[0];

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

	void makePayment(unique_ptr<DatabaseConnection<string>>& db, unique_ptr<PaymentStrategy>& p, double amount) {
		p->pay(amount);
	}

	void updateOrderStatus(unique_ptr<DatabaseConnection<string>>& db, int id, const string& status) override {
			throw runtime_error("Покупатель не может менять статус заказа");
	}

};

int main() {

	auto db = make_unique<DatabaseConnection<string>>(
			"host=localhost port=5432 dbname=online_store_db user=postgres password=Qwerty1234"
			);

	if (db->isConnected()) {
		cout << "Connected to database: " << db->getConnection().dbname() << endl;
	} else {
		cerr << "Failed to connect to the database." << endl;
		return 1;
	}

	return 0;
}
