CREATE TRIGGER trg_update_order_date
BEFORE UPDATE ON orders
FOR EACH ROW
EXECUTE FUNCTION update_order_date_on_status_change();