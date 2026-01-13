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
#include "../include/Order.h"
#include "../include/User.h"
#include "../include/Admin.h"
#include "../include/Manager.h"
#include "../include/Customer.h"

using namespace std;

int main() {

	unique_ptr<DatabaseConnection<string>> db = make_unique<DatabaseConnection<string>>("host=localhost port= dbname= password= user=");

	if(db->isConnected()) {
		cout << "Подключено успешно";
	}

	return 0;
}
