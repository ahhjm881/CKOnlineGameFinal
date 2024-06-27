

DELIMITER //

DROP PROCEDURE IF EXISTS GetItemsRange;

CREATE PROCEDURE GetItemsRange(IN p_id INT)
BEGIN

	SELECT *
	FROM ckdb.items
	WHERE item_id = p_id;

END //

DELIMITER ;

-- CALL GetItemsRange(23749);