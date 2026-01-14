#include "../include/Product.h"
using namespace std;

Product::Product(int id, const string& n, double p, int sq) : product_id(id), name(n), price(p), stock_quantity(sq) {}

int Product::getProductId() {
	return product_id;
}

string Product::getName() {
	return name;
}

double Product::getPrice() {
	return price;
}

int Product::getStock() {
	return stock_quantity;
}

shared_ptr<Product> ProductService::load(unique_ptr<DatabaseConnection<string>>& db, int product_id) {
	auto res = db->executeQuery("SELECT * FROM products\nWHERE product_id = " + to_string(product_id));

	if (res.empty())
	  throw runtime_error("Товар не найден");

	return make_shared<Product>(
		res[0]["product_id"].as<int>(),
		res[0]["name"].c_str(),
		res[0]["price"].as<double>(),
		res[0]["stock_quantity"].as<int>()
	);
}
