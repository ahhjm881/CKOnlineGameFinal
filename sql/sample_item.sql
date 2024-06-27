DELIMITER //

DROP PROCEDURE IF EXISTS SampleItem;

CREATE PROCEDURE SampleItem()
BEGIN
    DECLARE counter INT DEFAULT 1;
    DECLARE max_count INT DEFAULT 100000;

    -- 트랜잭션 시작
    START TRANSACTION;

    -- 반복문 시작
    my_loop: LOOP
        -- 반복문 내에서 수행할 작업
        IF counter > max_count THEN
            LEAVE my_loop; -- 반복문 종료 조건
        END IF;

        -- 예시 작업: 출력문
        INSERT INTO ckdb.items(item_name, item_desc)
		VALUES(CONCAT('item_', counter), CONCAT('this is item_', counter, '.'));
        

        SET counter = counter + 1; -- 카운터 증가
    END LOOP my_loop;

    -- 트랜잭션 커밋
    COMMIT;
END //

DELIMITER ;

CALL SampleItem();