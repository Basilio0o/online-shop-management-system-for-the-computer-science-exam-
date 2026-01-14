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
		COUNT(DISTINCT h.history_id) AS number_of_status_changes,
		la.operation,
		la.performed_at,
		COUNT(DISTINCT a1.log_id) AS number_of_audit_actions

	FROM orders o
	LEFT JOIN order_status_history h ON h.order_id = o.order_id
	LEFT JOIN last_audit la ON la.entity_id = o.order_id
	LEFT JOIN audit_log a ON a.order_id = o.order_id AND a.entity_type = 'order'
	GROUP BY o.order_id, o.status, o.order_date, la.operation, la.performed_at
	ORDER BY o.order_id;
END;
$$;