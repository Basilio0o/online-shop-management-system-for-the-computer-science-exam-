#include "../include/DatabaseConnection.h"
#include "../include/Product.h"
#include "../include/OrderItem.h"
#include "../include/Order.h"
#include "../include/User.h"
#include "../include/Admin.h"
#include "../include/Manager.h"
#include "../include/Customer.h"
#include "../include/Payment.h"

using namespace std;

void Menu() {
	cout << "=== ВЫБЕРИТЕ РОЛЬ ===" << endl;
	cout << "1. Войти как Администратор" << endl;
	cout << "2. Войти как Менеджер" << endl;
	cout << "3. Войти как Покупатель" << endl;
	cout << "0. Выход" << endl;
	cout << "Ваш выбор: ";
}

int main() {
	string login = "postgres";
	cout << "Введите пароль: ";
	string password;
	cin >> password;
	string connection = "host=localhost port=5432 dbname=online_store_db user= " + login + " password=" + password;

	unique_ptr<DatabaseConnection<string>> db = make_unique<DatabaseConnection<string>>(connection);

	int choice;

	Menu();

	cin >> choice;

	switch(choice) {
		case 1 : {
			cout << "Теперь вы администратор";
			unique_ptr<User> admin = make_unique<Admin>(db, 5, "Админ", "admin@mail.com", 1);

			dynamic_cast<Admin*>(admin.get())->runAdminMenu(db);

			break;
		}
		case 2: {
			cout << "Теперь вы менеджер";
			unique_ptr<User> manager = make_unique<Manager>(db, 3, "Олег Кузнецов", "oleg.k@mail.com", 0);

			dynamic_cast<Manager*>(manager.get())->runManagerMenu(db);

			break;
		}
		case 3 : {
			cout << "Теперь вы покупатель";

			unique_ptr<User> customer = make_unique<Customer>(db, 1, "Иван Петров", "ivan.petrov@mail.com", 0);

			dynamic_cast<Customer*>(customer.get())->runCustomerMenu(db);

			break;
		}
		case 0 : {
			cout << "Выход из программы...";
			break;
		}
		default : {
			cout << "Ошибка выбора. Попробуйте ещё раз" << endl;
		}
	}

	return 0;
}
