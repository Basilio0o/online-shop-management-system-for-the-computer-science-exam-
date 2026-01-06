CREATE OR REPLACE FUNCTION update_order_date_on_status_change()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
BEGIN
	IF NEW.status IS DISTINCT FROM OLD.status THEN
		NEW.order_date := NOW();
	END IF;

	RETURN NEW;
END;
$$;

CREATE OR REPLACE FUNCTION getOrderStatusHistory(oi INT)
RETURNS TABLE (
	history_id INT,
	order_id INT,
	old_status VARCHAR(20),
	new_status VARCHAR(20),
	changed_at TIMESTAMP,
	changed_by INT
)
LANGUAGE plpgsql
AS $$
BEGIN
	PERFORM 1
	FROM orders
	WHERE order_id = oi;

	IF NOT FOUND THEN
		RAISE EXCEPTION 'Заказ с id % не найден', oi;
	END IF;
		
	RETURN QUERY
	SELECT * FROM order_status_history
	WHERE order_id = oi
	ORDER BY changed_at ASC;
END;
$$;

CREATE OR REPLACE FUNCTION getOrderStatus(oi INT)
RETURNS VARCHAR(20)
LANGUAGE plpgsql
AS $$
DECLARE 
	s VARCHAR(20);
BEGIN
	
	SELECT status
	INTO s
	FROM orders
	WHERE order_id = oi;

	IF NOT FOUND THEN
		RAISE EXCEPTION 'Заказ с id % не найден', oi;
	END IF;

	RETURN s;

END;
$$;

CREATE OR REPLACE FUNCTION getUserOrderCount(ui INT)
RETURNS TABLE (
	status VARCHAR(20),
	count INT
)
LANGUAGE plpgsql
AS $$
BEGIN
	PERFORM 1
	FROM users
	WHERE user_id = ui;
  
	IF NOT FOUND THEN
		RAISE EXCEPTION 'Пользователь с id % не найден', ui;
	END IF;
	
	RETURN QUERY
	SELECT 
		status,
		COUNT(user_id) AS count
	FROM orders
	WHERE user_id = ui
	GROUP BY (status)
	ORDER BY status ASC;
END;
$$;

CREATE OR REPLACE FUNCTION getTotalSpentByUser(ui INT)
RETURNS DECIMAL(10,2)
LANGUAGE plpgsql
AS $$
DECLARE 
	t DECIMAL(10,2);
BEGIN

	PERFORM 1
	FROM users
	WHERE user_id = ui;
  
	IF NOT FOUND THEN
		RAISE EXCEPTION 'Пользователь с id % не найден', ui;
	END IF;
	
	SELECT 
		COALESCE(SUM(total_price), 0)
	INTO t
	FROM orders
	WHERE user_id = ui AND status = 'completed';

	RETURN t;
END;
$$;

CREATE OR REPLACE FUNCTION canReturnOrder(oi INT)
RETURNS BOOLEAN
LANGUAGE plpgsql
AS $$
DECLARE
	s VARCHAR(20);
	d TIMESTAMP;
BEGIN
	SELECT 
		status,
		order_date
	INTO s, d
	FROM orders
	WHERE order_id = oi;

	IF NOT FOUND THEN
		RAISE EXCEPTION 'Заказ с id % не найден', oi;
	END IF;

	IF s = 'completed' AND (NOW() - d <= INTERVAL '30 days') THEN
		RETURN TRUE;	
	END IF;

	RETURN FALSE;
END;
$$;
