/****************************************************************************
** Copyright (C) 2023 Ruslan_gold.
**
** Модуль, описывающий класс Server, который реализует работу сервера для
** разбора, мониторинга изменений и сохранения дынных из файлов .xml для
** дальнейшего взаимодействия этих данных с клиентом.
**
****************************************************************************/

#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QFileInfoList>
#include <QDir>
#include <QXmlStreamReader>
#include <QCryptographicHash>
#include <QTimer>
#include <QtSql>
#include <QSqlDatabase>
#include <iostream>

#include "sqlliteDB.h"
#include "sqlQueries.h"

using namespace std;

class SqlLiteDb;

class Server : public QTcpServer {
    Q_OBJECT
public:
    QTcpSocket *socket;
    Server();
    Server(QString dirForFiles, QString pathFileDb);
    void runServer();

private:
    QVector <QTcpSocket*> Sockets;
    QByteArray Data;
    QString dirForFiles; // путь до каталога с файлами .xml
    SqlLiteDb db; // экземпляр класса для работы с БД SQLLite

    void SendToClient(QVector<QVector<QString>> vector);

    QFileInfoList getFiles();
    void workWithFiles(QFileInfoList files);
    bool workWithOneFile(QString patfFile);
    QVector<QMap<QString, QMap<QString, QString>>> parsingXml(QString pathFile);
    QMap<QString, QString> parsingXmlBlock(QXmlStreamAttributes attrib);
    QVector<QMap<QString, QMap<QString, QString>>> createGoodData(QVector<QMap<QString, QMap<QString, QString>>> data);
    QString getHashFile(QString pathFile);


public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
    void onTimeout();


};

#endif // SERVER_H
