#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QFileInfoList>
#include <QDir>
#include <iostream>

using namespace std;

class Server : public QTcpServer {
    Q_OBJECT
public:
    QTcpSocket *socket;

    Server();
    Server(QString dirForFiles);
    void runServer();

private:
    QVector <QTcpSocket*> Sockets;
    QByteArray Data;
    QString dirForFiles;
    void SendToClient(QString str);
    QFileInfoList getFiles();
    void parsingXml(QFileInfoList files);


public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
    void onTimeout();


};

#endif // SERVER_H
