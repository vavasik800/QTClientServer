/****************************************************************************
** Copyright (C) 2023 Ruslan_gold.
**
** Модуль описывает класс SqlLiteDb, предназначенный для взаимодействия с
** СУБД SqlLite, для отправки запросов.
**
****************************************************************************/

#ifndef SQLLITEDB_H
#define SQLLITEDB_H

#include <QVector>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QFileInfo>
#include <iostream>

#include "sqlQueries.h" // файл с шаблонами для запросов

using namespace std;


class SqlLiteDb : public QSqlDatabase {

public:
    SqlLiteDb() {};
    SqlLiteDb(QString pathFile);
    bool testRunBd();
    void setPath(QString pathFile);
    QVector<QMap<QString, QString>> runQuerySelect(QString query);
    bool runQueryInsert(QString query);
    bool runQueryUpdate(QString query);
    bool runQueryInsert(QString tableName, QMap<QString, QString> data);
    bool runQueryDelete(QString query);
    QVector<QMap<QString, QString>> runQuery(QString query, QString typeQuery);
    QString createQuery(QString tableName, QMap<QString, QString> data);

private:
    QString pathFile; // путь до файла с БД
    QSqlDatabase sdb; // экземпляр класса для обращения с БД
    bool runBd; // переменная для проверки работы БД

};

#endif // SQLLITEDB_H
