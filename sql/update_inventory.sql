DELIMITER //

DROP PROCEDURE IF EXISTS InventoryUpdate;

CREATE PROCEDURE InventoryUpdate(IN p_table_name VARCHAR(1024), IN p_item_id INT, IN p_item_count INT, p_is_Set BOOL)
BEGIN
    DECLARE d_item_id INT;
    DECLARE sql_query VARCHAR(500);

	set @d_item_id = NULL;

    -- 트랜잭션 시작
    START TRANSACTION;

    -- 해당 아이템 ID에 대한 레코드를 SELECT하여 FOR UPDATE로 Lock을 걸고 가져옵니다.
    SET @sql_query = CONCAT('SELECT item_id INTO @d_item_id FROM inventory.', p_table_name, ' WHERE item_id = ', p_item_id, ' FOR UPDATE');
    PREPARE stmt FROM @sql_query;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    -- d_item_id 변수가 NULL이 아닌 경우 UPDATE 문을 실행하고, 아닌 경우 INSERT 문을 실행합니다.
    IF @d_item_id IS NOT NULL THEN
		IF p_is_Set IS TRUE THEN
			SET @sql_query = CONCAT('UPDATE inventory.', p_table_name, ' SET item_count = ', p_item_count, ' WHERE item_id = ', p_item_id);
        ELSE
			SET @sql_query = CONCAT('UPDATE inventory.', p_table_name, ' SET item_count = item_count + ', p_item_count, ' WHERE item_id = ', p_item_id);
        END IF;
    ELSE
        SET @sql_query = CONCAT('INSERT INTO inventory.', p_table_name, '(item_id, item_count) VALUES(', p_item_id, ', ', p_item_count, ')');
    END IF;

    PREPARE stmt FROM @sql_query;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    -- 트랜잭션 커밋
    COMMIT;
END //

DELIMITER ;

CALL InventoryUpdate("query_tester", 32, 11, true);