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
	FROM orders o
	WHERE o.order_id = oi;

	IF NOT FOUND THEN
		RAISE EXCEPTION 'Заказ с id % не найден', oi;
	END IF;
		
	RETURN QUERY
	SELECT * FROM order_status_history oh
	WHERE oh.order_id = oi
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
	FROM orders o
	WHERE o.order_id = oi;

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
	FROM users u
	WHERE u.user_id = ui;
  
	IF NOT FOUND THEN
		RAISE EXCEPTION 'Пользователь с id % не найден', ui;
	END IF;
	
	RETURN QUERY
	SELECT 
		status,
		COUNT(user_id) AS count
	FROM orders o
	WHERE o.user_id = ui
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
	FROM users u
	WHERE u.user_id = ui;
  
	IF NOT FOUND THEN
		RAISE EXCEPTION 'Пользователь с id % не найден', ui;
	END IF;
	
	SELECT 
		COALESCE(SUM(total_price), 0)
	INTO t
	FROM orders o
	WHERE o.user_id = ui AND o.status = 'completed';

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
	FROM orders o
	WHERE o.order_id = oi;

	IF NOT FOUND THEN
		RAISE EXCEPTION 'Заказ с id % не найден', oi;
	END IF;

	IF s = 'completed' AND (NOW() - d <= INTERVAL '30 days') THEN
		RETURN TRUE;	
	END IF;

	RETURN FALSE;
END;
$$;

CREATE OR REPLACE FUNCTION getAuditLogByUser(ui INT)
RETURNS TABLE (
	log_id INT,
	entity_type VARCHAR(20),
	entity_id INT,
	operation VARCHAR(20),
	performed_by INT,
	performed_at TIMESTAMP
)
LANGUAGE plpgsql
AS $$
BEGIN
	PERFORM 1
	FROM users u
	WHERE u.user_id = ui;

	IF NOT FOUND THEN
		RAISE EXCEPTION 'Пользователь с id % не найден', ui;
	END IF;

	RETURN QUERY
	SELECT * FROM audit_log a
	WHERE a.performed_by = ui
	ORDER BY performed_at ASC;
END;
$$;

CREATE OR REPLACE FUNCTION createOrderAuditHistory ()
RETURNS TABLE (
	order_id INT,
	current_status VARCHAR(20),
	order_date TIMESTAMP,
	number_of_status_changes INT,
	last_action VARCHAR(20),
	action_date TIMESTAMP,
	number_of_audit_actions INT
)
LANGUAGE plpgsql
AS $$
BEGIN
	RETURN QUERY
	WITH last_audit AS (
		SELECT DISTINCT ON(entity_id)
			entity_id,
			operation,
			performed_at
		FROM audit_log a
		WHERE a.entity_type = 'order'
		ORDER BY entity_id, performed_at DESC
	)
	
	SELECT
		o.order_id,
		o.status AS current_status,
		o.order_date,
		CAST(COUNT(DISTINCT h.history_id) AS INT) AS number_of_status_changes,
		la.operation,
		la.performed_at,
		CAST(COUNT(DISTINCT a.log_id) AS INT) AS number_of_audit_actions

	FROM orders o
	LEFT JOIN order_status_history h ON h.order_id = o.order_id
	LEFT JOIN last_audit la ON la.entity_id = o.order_id
	LEFT JOIN audit_log a ON a.entity_id = o.order_id AND a.entity_type = 'order'
	GROUP BY o.order_id, o.status, o.order_date, la.operation, la.performed_at
	ORDER BY o.order_id;
END;
$$;
