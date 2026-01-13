#pragma once

using namespace std;

#include <iostream>
#include <memory>
#include <string>
#include "../include/DatabaseConnection.h"

class Product {
private:
	int product_id;
	string name;
	double price;
	int stock_quantity;
public:
	Product(int id, const string& n, double p, int sq);

	int getId();

	string getName();

	double getPrice();

	int getStock();
};

class ProductService {
public:
	static shared_ptr<Product> load(std::unique_ptr<DatabaseConnection<string>>& db, int product_id);
};
