
use ckdb;

DELIMITER //

DROP PROCEDURE IF EXISTS GetInventoryItemList;

CREATE PROCEDURE GetInventoryItemList(IN p_table_name VARCHAR(1024))
BEGIN
    DECLARE sql_query VARCHAR(500);

    -- 트랜잭션 시작
    START TRANSACTION;

    -- 해당 아이템 ID에 대한 레코드를 SELECT하여 FOR UPDATE로 Lock을 걸고 가져옵니다.
    SET @sql_query = CONCAT('SELECT * FROM inventory.', p_table_name, ';');
    
    PREPARE stmt FROM @sql_query;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;

    -- 트랜잭션 커밋
    COMMIT;
END //

DELIMITER ;

-- CALL TryCreateInventoryTable("query_tester");
CALL GetInventoryItemList("query_tester");