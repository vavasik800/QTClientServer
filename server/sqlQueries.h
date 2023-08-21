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

#define CREATE_TABLE_FILES "CREATE TABLE files (id   INTEGER PRIMARY KEY AUTOINCREMENT,\
                                                name TEXT,\
                                                path TEXT,\
                                                hash TEXT);"

#define CREATE_TABLE_BLOCK "CREATE TABLE block (uid         INTEGER PRIMARY KEY AUTOINCREMENT,\
                                                id          INTEGER UNIQUE,\
                                                Name        TEXT,\
                                                MtR         TEXT,\
                                                MtC         TEXT,\
                                                Label       TEXT,\
                                                IP          TEXT,\
                                                Description TEXT,\
                                                BoardCount  INTEGER);"

#define CREATE_TABLE_BOARD "CREATE TABLE board (uid       INTEGER PRIMARY KEY AUTOINCREMENT,\
                                                id        INTEGER UNIQUE,\
                                                Algoritms TEXT,\
                                                IntLinks  TEXT,\
                                                Name      TEXT,\
                                                Num       INTEGER,\
                                                PortCount INTEGER,\
                                                id_block  INTEGER REFERENCES block (id) ON DELETE CASCADE);"

#define CREATE_TABLE_PORT "CREATE TABLE port (uid      INTEGER PRIMARY KEY AUTOINCREMENT,\
                                              id       INTEGER,\
                                              Num      TEXT,\
                                              Media    TEXT,\
                                              Signal   TEXT,\
                                              id_board INTEGER REFERENCES board (id) ON DELETE CASCADE);"


#define TABLES_FOR_SEARCH "block,board,port"

#endif // SQLQUERIES_H
