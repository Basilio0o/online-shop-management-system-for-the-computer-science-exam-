#include "../include/OrderItem.h"
#include "../include/Product.h"

OrderItem::OrderItem(shared_ptr<Product>& p, int q) : product(p), quantity(q) {}

double OrderItem::getTotal_item(){
	return quantity * product->getPrice();
}

int OrderItem::getProductId(){
	return product->getProductId();
}

int OrderItem::getQuantity(){
	return quantity;
}
