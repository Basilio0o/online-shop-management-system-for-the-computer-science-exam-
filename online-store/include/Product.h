#pragma once
#include "../include/DatabaseConnection.h"
#include <iostream>
#include <memory>
#include <string>

class Product {
private:
	int product_id;
	std::string name;
	double price;
	int stock_quantity;
public:
	Product(int id, const std::string& n, double p, int sq);

	int getProductId();

	std::string getName();

	double getPrice();

	int getStock();
};

class ProductService {
public:
	static std::shared_ptr<Product> load(std::unique_ptr<DatabaseConnection<std::string>>& db, int product_id);
};
