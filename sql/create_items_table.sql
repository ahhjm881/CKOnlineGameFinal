
use ckdb;

DROP TABLE IF EXISTS Items;

CREATE TABLE Items(
item_id int AUTO_INCREMENT NOT NULL UNIQUE PRIMARY KEY,
item_name VARCHAR(40) NOT NULL,
item_desc VARCHAR(1024)
);

