#ifndef SQLQUERIES_H
#define SQLQUERIES_H

// шаблоны для конструирования запроса на вставку данных
#define BEGIN_INSERT_QUERY "INSERT INTO %1("
#define VALUES_INSERT_QUERY ") VALUES ("
#define END_INSERT_QUERY ");"

// запрос на вставку файла в таблицу files
#define INSERT_FILE "INSERT INTO files(name, path, hash) VALUES ('%1', '%2', '%3');"
// запрос на получение данных из таблицы files
#define SELECT_FILE "SELECT id, name, hash from files where name = '%1';"
// запрос на обновление хэша файла
#define UPDATE_HASH_FILE "UPDATE files set hash = '%1' where name = '%2';"
// запрос на удаление данных из таблицы block
#define DELETE_CASKADE_FILE "DELETE from block where IP='%1';"

#define SELECT_BLOCK "select id, Name,MtR,MtC,Label,IP,Description,BoardCount from block"
#define SELECT_ALL "select * from %1"


#define TABLES_FOR_SEARCH "block,board,port"

#endif // SQLQUERIES_H
