DELIMITER //

DROP PROCEDURE UpdateData;
CREATE PROCEDURE UpdateData(IN p_name CHAR(40), IN p_item_name CHAR(40), IN p_item_count int)
BEGIN
    DECLARE d_name VARCHAR(40);
    DECLARE d_item_name VARCHAR(40);
    DECLARE d_item_count int;
    
    
    START TRANSACTION;
    
    SELECT name, item_name, item_count
    INTO d_name, d_item_name, d_item_count
    FROM ckdb.inventory
    WHERE name = p_name and item_name = p_item_name
    FOR UPDATE;
    
    IF d_name IS NOT NULL and d_item_name IS NOT NULL THEN
        UPDATE ckdb.inventory
        SET item_count = p_item_count
        WHERE name = p_name and p_item_name = d_item_name;
    ELSE
		INSERT INTO inventory(name, item_name, item_count)
        VALUES(p_name, p_item_name, p_item_count);
    END IF;
    
    
	COMMIT;
END //

DELIMITER ;