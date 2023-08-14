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
    //    onTimeout();
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(10000);
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

void Server::workWithFiles(QFileInfoList files)
{
    for (int i = 0; i < files.size(); ++i) {
        QFileInfo fileInfo = files.at(i);
        QString nameFile = fileInfo.fileName();
        cout << qPrintable(nameFile) << endl;
        QString patfFile = QString("%1/%2").arg(fileInfo.path()).arg(nameFile);
        QString hashFile = getHashFile(patfFile);

        QString query = SELECT_FILE;
        query = query.arg(nameFile);

        QVector<QVector<QString>> result = db.runQuerySelect(query);
        if (result.size() > 1) {
            cout << "Check DataBase. Double files" << endl;
            return;
        }
        if (result.size() == 0){
            // парсинг файла
            auto dataFromFile = parsingXml(patfFile);
            auto newDataWithId = createGoodData(dataFromFile);
            query = QString(INSERT_FILE).arg(nameFile).arg(fileInfo.path()).arg(hashFile);
            if(!db.runQueryInsert(query))
                cout << "Bad Insert!" << endl;
            for (auto elem: newDataWithId) {
                if(!db.runQueryInsert(elem.keys()[0], elem.value(elem.keys()[0])))
                    cout << "Bad Insert!" << endl;
            }
        }
        else if (result[0][2] == hashFile) {
            cout << "Files in BD, Don't shanges." << endl;
            continue;
        }
        else {
            // парсинг файла
            auto dataFromFile = parsingXml(patfFile);
            auto newDataWithId = createGoodData(dataFromFile);
            // обновление хэша
            query = QString(UPDATE_HASH_FILE).arg(hashFile).arg(nameFile);
            if(!db.runQueryUpdate(query))
                cout << "Bad Update!" << endl;
            for (auto elem: newDataWithId) {
                if(!db.runQueryInsert(elem.keys()[0], elem.value(elem.keys()[0])))
                    cout << "Bad Insert!" << endl;
            }
        }
    }
}

QVector<QMap<QString, QMap<QString, QString>>> Server::parsingXml(QString pathFile)
{
    QVector<QMap<QString, QMap<QString, QString>>> result;
    QFile* file = new QFile(pathFile);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        cout << "error file" << endl;
        return result;
    }
    QXmlStreamReader xml(file);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        QMap<QString, QMap<QString, QString>> element;
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "net")
                continue;
            QMap<QString, QString> elem = parsingXmlBlock(xml.attributes());
            element[xml.name().toString()] = elem;
        }
        if (element.size() != 0)
            result.push_back(element);
    }
    return result;
}

QMap<QString, QString> Server::parsingXmlBlock(QXmlStreamAttributes attrib)
{
    QMap<QString, QString> elem;
    for (int i = 0; i < attrib.size(); i++){
        elem[attrib[i].name().toString()] = attrib[i].value().toString();
    }
    return elem;

}

QVector<QMap<QString, QMap<QString, QString>>> Server::createGoodData(QVector<QMap<QString, QMap<QString, QString>>> data)
{
    QVector<QMap<QString, QMap<QString, QString>>> result;
    QVector<QString> tags;
    QVector<QString> idsTags;
    for (auto elem: data) {
        QMap<QString, QMap<QString, QString>> row = elem;
        QString tag = elem.keys()[0];
        QString id = elem.value(elem.keys()[0]).value("id");
        if (tags.size() == 0){
            tags.push_back(tag);
            idsTags.push_back(id);
            result.push_back(row);
            continue;
        }
        int indexTag = tags.indexOf(tag);
        if (indexTag != -1) {
            tags = tags.mid(0, indexTag);
            idsTags = idsTags.mid(0, indexTag);
        }
        QString tagUp = tags.last();
        QString idUp = idsTags.last();
        auto newMap = elem.value(elem.keys()[0]);
        newMap[QString("id_")+tagUp] = idUp;
        QMap<QString, QMap<QString, QString>> newRow;
        newRow[tag] = newMap;
        tags.push_back(tag);
        idsTags.push_back(id);
        result.push_back(newRow);
    }
    return result;
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
    cout << "Start read files" << endl;
    QFileInfoList files = this->getFiles();
    cout << "Finish read files" << endl;
    workWithFiles(files);
    cout << endl;
}

