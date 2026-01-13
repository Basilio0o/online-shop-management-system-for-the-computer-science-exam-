#pragma once

using namespace std;

#include "../include/Product.h"
#include <memory>

class OrderItem {
private:
	shared_ptr<Product> product;
	int quantity;
public:
	OrderItem(shared_ptr<Product>& p, int q);

	double getTotal_item();

	int getProductId();

	int getQuantity();
};

