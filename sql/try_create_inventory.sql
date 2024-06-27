DELIMITER //

DROP PROCEDURE IF EXISTS TryCreateInventoryTable;
CREATE DATABASE IF NOT EXISTS inventory;

CREATE PROCEDURE TryCreateInventoryTable(IN p_table_name VARCHAR(1024))
BEGIN
	SET @query = CONCAT('CREATE TABLE IF NOT EXISTS inventory.',  p_table_name, '(
    auto_id int AUTO_INCREMENT PRIMARY KEY,
    item_id int UNIQUE,
    item_count int,
    FOREIGN KEY(item_id) REFERENCES ckdb.Items(item_id)
);');

PREPARE stmt FROM @query;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

END //

DELIMITER ;

CALL TryCreateInventoryTable("query_tester");