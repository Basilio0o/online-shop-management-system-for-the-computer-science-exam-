#pragma once
#include "../include/Product.h"
#include <memory>

class OrderItem {
private:
	std::shared_ptr<Product> product;
	int quantity;
public:
	OrderItem(const std::shared_ptr<Product>& p, int q);

	double getTotal_item();

	int getProductId();

	int getQuantity();
};

