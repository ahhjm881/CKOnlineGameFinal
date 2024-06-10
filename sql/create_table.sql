create database ckdb;
use ckdb;

create table inventory(
name char(40) not null,
item_name char(40) not null,
item_count int not null
);