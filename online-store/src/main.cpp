#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>

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

class User {
private:
	int user_id;
	string name;
	string email;
	string role;
	string password;
	int loyalty_level;
public:
	User(unique_ptr<DatabaseConnection<string>>& db, int u, const string& n,
			const string& e, const string& r, const string& p, int l) :
				user_id(u), email(e), role(r), password(p), loyalty_level(l)
	{
		string query = "SET app.current_user_id = " + db->getConnection().quote(user_id) + ");";

		db->executeNonQuery(query);
	}

	virtual ~User();

	string buildOrderItemArray(pqxx::work& txn, const vector<pair<int, int>>& list) {
		if (list.empty()) {
			throw runtime_error("Ваш список товаров пуст");
		}
		string result = "ARRAY[";
		for(size_t i = 0; i < list.size(); i++) {
			result += "(" +
					txn.quote(list[i].first) + "," +
					txn.quote(list[i].second) + ")";
			if (i + 1 < list.size()) {
				result += ", ";
			}

		}
		result += "]::order_items_t[]";
		return result;
	}

	int createOrder(unique_ptr<DatabaseConnection<string>>& db, const vector<pair<int, int>>& list) {
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

	auto u = make_unique<User>(db, 1, "Иван Петров", "ivan.petrov@mail.com", "customer", "hash_ivan", 0);

	u->cancelOrder(db, 26);

	return 0;
}
