#include "server.h"
#include "sqlliteDB.h"


Server::Server() {
    if(this->listen(QHostAddress::Any, 2323))
        qDebug() << "start";
    else
        qDebug() << "error";

}

Server::Server(QString dirForFiles, QString pathFileDb) {
    this->dirForFiles = dirForFiles;
    db.setPath(pathFileDb);
}

void Server::runServer()
{
    cout << "Server run" << endl;
    cout << qPrintable(dirForFiles) << endl;
    QFileInfoList files = this->getFiles();
    for (int i = 0; i < files.size(); ++i) {
        QFileInfo fileInfo = files.at(i);
        QString patfFile = QString("%1/%2").arg(fileInfo.path()).arg(fileInfo.fileName());
        QString hashFile = getHashFile(patfFile);

        QString query = SELECT_FILE;
        query = query.arg(fileInfo.fileName() + QString("dd"));

        QVector<QVector<QString>> result = db.runQuerySelect(query);
        cout << result.size() << endl;
    }

//    QVector<QString> a = db.runQuerySelect(query);
//    QTimer* timer = new QTimer(this);
//    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
//    timer->start(10000);
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

        QString str_insert = "INSERT INTO files(name, path, hash) "
                             "VALUES ('%1', '%2', '%3');";

        str_insert = str_insert.arg(fileInfo.fileName())
                .arg(fileInfo.path());

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
//                            cout << "block: ";
                             QXmlStreamAttributes attrib = xml.attributes();
                             for (int i = 0; i < attrib.size(); i++){
                                 QString s = QString("%1: %2; ").arg(attrib[i].name()).arg(attrib[i].value());
//                                 cout << qPrintable(s);
                                 alldata += s;
                             }
//                             cout << endl;
                        }
                        if (xml.name() == "board"){
//                            cout << "\tboard: ";
                             QXmlStreamAttributes attrib = xml.attributes();
                             for (int i = 0; i < attrib.size(); i++){
                                 QString s = QString("%1: %2; ").arg(attrib[i].name()).arg(attrib[i].value());
//                                 cout << qPrintable(s);
                                 alldata += s;
                             }
//                             cout << endl;
                        }
                        if (xml.name() == "port"){
//                            cout << "\t\tport: ";
                             QXmlStreamAttributes attrib = xml.attributes();
                             for (int i = 0; i < attrib.size(); i++){
                                 QString s = QString("%1: %2; ").arg(attrib[i].name()).arg(attrib[i].value());
//                                 cout << qPrintable(s);
                                 alldata += s;
                             }
//                             cout << endl;
                        }
                    }
                }
        QString hash = QString("%1").arg(QString(QCryptographicHash::hash(alldata.toUtf8(),QCryptographicHash::Sha1).toHex()));
        str_insert = str_insert.arg(hash);
        bool resilt = db.runQueryInsert(str_insert);
        cout << resilt << endl;
    }
}

QString Server::getHashFile(QString pathFile)
{
    QString hashStr = "";
    QFile f(pathFile);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        if (hash.addData(&f)) {
            hashStr += QString(hash.result().toHex());
        }
    }
    return hashStr;
}

