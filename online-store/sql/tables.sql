CREATE TABLE users (
	user_id SERIAL PRIMARY KEY,
	name VARCHAR(100) NOT NULL,
	email VARCHAR(100) UNIQUE NOT NULL,
	role VARCHAR(20) NOT NULL CHECK (role IN ('admin', 'manager', 'customer')),
	password_hash VARCHAR(255) NOT NULL,
	loyalty_level INT NOT NULL DEFAULT 0 CHECK (loyalty_level IN (0, 1))
);

CREATE TABLE products (
	product_id SERIAL PRIMARY KEY,
	name VARCHAR(100) NOT NULL,
	price DECIMAL(10,2) NOT NULL CHECK (price > 0),
	stock_quantity INT NOT NULL CHECK (stock_quantity >= 0)
);

CREATE TABLE orders (
	order_id SERIAL PRIMARY KEY,
	user_id INT,
	FOREIGN KEY (user_id) REFERENCES users(user_id),
	status VARCHAR(20) NOT NULL CHECK (status IN ('pending', 'completed', 'canceled', 'returned')),
	total_price DECIMAL(10,2) NOT NULL,
	order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE 	
	order_item_id SERIAL PRIMARY KEY,
	order_id INT,
	FOREIGN KEY (order_id) REFERENCES orders(order_id),
	product_id INT,
	FOREIGN KEY (product_id) REFERENCES products(product_id),
	quantity INT NOT NULL CHECK (quantity > 0),
	price DECIMAL(10,2) NOT NULL CHECK (price > 0)
);

CREATE TABLE order_status_history (
	history_id SERIAL PRIMARY KEY,
	order_id INT,
	FOREIGN KEY (order_id) REFERENCES orders(order_id),
	old_status VARCHAR(20),
	new_status VARCHAR(20) NOT NULL,
	changed_at TIMESTAMP,
	changed_by INT,
	FOREIGN KEY (changed_by) REFERENCES users(user_id)
);

CREATE TABLE audit_log (
	log_id SERIAL PRIMARY KEY,
	entity_type VARCHAR(20) NOT NULL CHECK (entity_type IN ('order', 'product', 'user')),
	entity_id INT NOT NULL,
	operation VARCHAR(20) NOT NULL CHECK (operation IN ('insert', 'update', 'delete')),
	performed_by INT,
	FOREIGN KEY (performed_by) REFERENCES users(user_id),
	performed_at TIMESTAMP
);

