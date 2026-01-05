CREATE OR REPLACE PROCEDURE createOrder (
	ui INT,
	pi INT,
	q INT
)
LANGUAGE plpgsql
AS $$
DECLARE 
	oi INT;
	tp DECIMAL(10,2) := 0;
	sq INT;
	p DECIMAL(10,2);
BEGIN
	PERFORM 1
	FROM users
	WHERE user_id = ui;
  
	IF NOT FOUND THEN
		RAISE EXCEPTION 'Пользователь с id % не найден', ui;
	END IF;
  
	INSERT INTO orders (user_id, status, total_price, order_date) 
	VALUES (ui, 'pending', 0, NOW())
	RETURNING order_id INTO oi;
	
	SELECT price, stock_quantity
	INTO p, sq
	FROM products 
	WHERE product_id = pi
	FOR UPDATE;

	IF NOT FOUND THEN
		RAISE EXCEPTION 'Товар с id % не найден', pi;
	END IF;

	IF q > sq THEN
		RAISE EXCEPTION 'Недостаточно товара на складе (product_id = %)', pi;
	END IF;

	INSERT INTO order_items (order_id, product_id, quantity, price)
	VALUES (oi, pi, q, p);

	UPDATE products 
	SET stock_quantity = sq - q
	WHERE product_id = pi;

	tp := p*q;

	UPDATE orders
	SET total_price = tp
	WHERE order_id = oi;

	INSERT INTO order_status_history (order_id, old_status, new_status, changed_at, changed_by)
	VALUES (oi, NULL, 'pending', NOW(), ui);
	
	INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at)
	VALUES ('product', pi, 'update', ui, NOW());
	
	INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at)
	VALUES ('order', oi, 'insert', ui, NOW());

END;
$$;
