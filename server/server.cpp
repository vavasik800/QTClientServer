#include "server.h"

#include <QXmlStreamReader>
#include <QCryptographicHash>
#include <QTimer>


Server::Server() {
    if(this->listen(QHostAddress::Any, 2323))
        qDebug() << "start";
    else
        qDebug() << "error";

}

Server::Server(QString dirForFiles) {
    this->dirForFiles = dirForFiles;
}

void Server::runServer()
{
    cout << "Server run" << endl;
    cout << qPrintable(dirForFiles) << endl;
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(10000);


}

void Server::incomingConnection(qintptr socketDescriptor){
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    Sockets.push_back(socket);
    qDebug() << "client connected" << socketDescriptor;
}


void Server::slotReadyRead(){
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_14);
    if (in.status() == QDataStream::Ok){
        qDebug() << "read...";
        QString str;
        in >> str;
        qDebug() << str;
    }
    else
        qDebug() << "DataStream error";
}

void Server::onTimeout()
{
    QFileInfoList files = this->getFiles();
    this->parsingXml(files);
}

void Server::SendToClient(QString str){
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_14);
    out << str;
    socket->write(Data);
}

QFileInfoList Server::getFiles()
{
    QDir dir;  //объявляем объект работы с папками
    dir.setFilter(QDir::Files);
    dir.cd(dirForFiles);
    QFileInfoList result = dir.entryInfoList();     //получаем список файлов директории
    return result;
}

void Server::parsingXml(QFileInfoList files)
{
    for (int i = 0; i < files.size(); ++i) {
        QFileInfo fileInfo = files.at(i);
        QString patfFile = QString("%1/%2").arg(fileInfo.path()).arg(fileInfo.fileName());
        cout << qPrintable(patfFile) << endl;
        QFile* file = new QFile(patfFile);
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            cout << "error file" << endl;
            continue;
        }
        QXmlStreamReader xml(file);
        QString alldata;
        while (!xml.atEnd() && !xml.hasError())
        {

            QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::StartDocument)
                continue;
            if (token == QXmlStreamReader::StartElement)
            {
                if (xml.name() == "net")
                    continue;
                if (xml.name() == "block"){
                    cout << "block: ";
                     QXmlStreamAttributes attrib = xml.attributes();
                     for (int i = 0; i < attrib.size(); i++){
                         QString s = QString("%1: %2; ").arg(attrib[i].name()).arg(attrib[i].value());
                         cout << qPrintable(s);
                         alldata += s;
                     }
                     cout << endl;
                }
                if (xml.name() == "board"){
                    cout << "\tboard: ";
                     QXmlStreamAttributes attrib = xml.attributes();
                     for (int i = 0; i < attrib.size(); i++){
                         QString s = QString("%1: %2; ").arg(attrib[i].name()).arg(attrib[i].value());
                         cout << qPrintable(s);
                         alldata += s;
                     }
                     cout << endl;
                }
                if (xml.name() == "port"){
                    cout << "\t\tport: ";
                     QXmlStreamAttributes attrib = xml.attributes();
                     for (int i = 0; i < attrib.size(); i++){
                         QString s = QString("%1: %2; ").arg(attrib[i].name()).arg(attrib[i].value());
                         cout << qPrintable(s);
                         alldata += s;
                     }
                     cout << endl;
                }
            }
        }
        QString hash = QString("%1").arg(QString(QCryptographicHash::hash(alldata.toUtf8(),QCryptographicHash::Sha1).toHex()));
        cout << qPrintable(hash) << endl;
        //1ef9dfb922fe398db9997973278e226cc468a87c
        //a0a44027b432749349f13c08f9230cca907c33b3
        //1ef9dfb922fe398db9997973278e226cc468a87c
    }
}

