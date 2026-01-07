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

CREATE OR REPLACE FUNCTION audit_log_users()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
DECLARE
    entity_id INT;
BEGIN
    IF TG_OP = 'DELETE' THEN
        entity_id := OLD.user_id;
    ELSE
        entity_id := NEW.user_id;
    END IF;

    INSERT INTO audit_log (
        entity_type,
        entity_id,
        operation,
        performed_by,
        performed_at
    )
    VALUES (
        'user',
        entity_id,
        LOWER(TG_OP),
        current_setting('app.current_user_id')::INT,
        now()
    );

    RETURN NULL;
END;
$$;

CREATE OR REPLACE FUNCTION audit_log_products()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
DECLARE
    entity_id INT;
BEGIN
    IF TG_OP = 'DELETE' THEN
        entity_id := OLD.product_id;
    ELSE
        entity_id := NEW.product_id;
    END IF;

    INSERT INTO audit_log (
        entity_type,
        entity_id,
        operation,
        performed_by,
        performed_at
    )
    VALUES (
        'product',
        entity_id,
        LOWER(TG_OP),
        current_setting('app.current_user_id')::INT,
        now()
    );

    RETURN NULL;
END;
$$;

CREATE OR REPLACE FUNCTION audit_log_orders()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
DECLARE
    entity_id INT;
BEGIN
    IF TG_OP = 'DELETE' THEN
        entity_id := OLD.order_id;
    ELSE
        entity_id := NEW.order_id;
    END IF;

    INSERT INTO audit_log (
        entity_type,
        entity_id,
        operation,
        performed_by,
        performed_at
    )
    VALUES (
        'order',
        entity_id,
        LOWER(TG_OP),
        current_setting('app.current_user_id')::INT,
        now()
    );

    RETURN NULL;
END;
$$;
