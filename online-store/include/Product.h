#pragma once

using namespace std;

#include "../include/DatabaseConnection.h"
#include <iostream>
#include <memory>
#include <string>

class Product {
private:
	int product_id;
	string name;
	double price;
	int stock_quantity;
public:
	Product(int id, const string& n, double p, int sq);

	int getProductId();

	string getName();

	double getPrice();

	int getStock();
};

class ProductService {
public:
	static shared_ptr<Product> load(std::unique_ptr<DatabaseConnection<string>>& db, int product_id);
};
