#include "server.h"
#include "sqlliteDB.h"


Server::Server() { }


Server::Server(QString dirForFiles, QString pathFileDb) {
    this->dirForFiles = dirForFiles;
    db.setPath(pathFileDb);
    if(this->listen(QHostAddress::Any, 2323))
        qDebug() << "Server run";
    else
        qDebug() << "Server error";
}

// Основной метод, осуществляющий работу сервера.
// Проверяет файлы в каталоге на изменения каждые 20 секунд.
void Server::runServer()
{
    onTimeout();
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(20000);
}

// Метод отправки данных на клиента
void Server::SendToClient(QMap<QString, QVector<QMap<QString, QString>>> mapResponse){
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_14);
    out << mapResponse;
    socket->write(Data);
}

QFileInfoList Server::getFiles()
{
    QDir dir; //объявляем объект работы с папками
    dir.setFilter(QDir::Files);
    dir.cd(dirForFiles);
    QFileInfoList result = dir.entryInfoList(); //получаем список файлов директории
    for (auto file: result){
        // просмотр всех файлов на случай если есть не .xml
        if (file.fileName().mid(file.fileName().size() - 4) != ".xml") {
            cout << "The directory either does not exist or contains more than .xml files!" << endl;
            result = QFileInfoList();
            return result;
        }
    }
    return result;
}

// Метод для работы с файлами .xml.
// Здесь файлы парсятся, а затем данные сохраняются в БД.
//  :param files: набор файлов для обработки
void Server::workWithFiles(QFileInfoList files)
{
    for (int i = 0; i < files.size(); ++i) {
        QFileInfo fileInfo = files.at(i);
        QString nameFile = fileInfo.fileName(); // название файла
        cout << qPrintable(nameFile) << endl;
        QString patfFile = QString("%1/%2").arg(fileInfo.path()).arg(nameFile);
        QString hashFile = getHashFile(patfFile); // получение Хэша файла

        QString query = SELECT_FILE;
        query = query.arg(nameFile);
        // запрос данных о наличии данных файлов В БД
        QVector<QMap<QString, QString>> result = db.runQuery(query, "select");
        if (result.size() > 1) {
            cout << "Check DataBase. Double files" << endl;
            return;
        }
        if (result.size() == 0){
            // Случай, когда файла нет в БД.
            query = QString(INSERT_FILE).arg(nameFile).arg(fileInfo.path()).arg(hashFile);
            db.runQuery(query, "insert"); // добавление файла и его хэша
            // обработка файла
            if (!workWithOneFile(patfFile)) {
                cout << "File '%1' is bad! Check file, please!" << endl;
                continue;
            }
        }
        else if (result[0]["hash"] == hashFile) {
            // Случай, когда файл не изменялся (хэши совпадают)
            cout << "Files in BD, Don't shanges." << endl;
            continue;
        }
        else {
            // Случай, когда файл изменился.
            auto ip = nameFile.mid(0, nameFile.size() - 4);
            query = QString(DELETE_CASKADE_FILE).arg(ip);
            db.runQuery(query, "delete"); // каскадное удаление данных связанных с измененнным файлом
            // обновление хэша
            query = QString(UPDATE_HASH_FILE).arg(hashFile).arg(nameFile);
            db.runQuery(query, "update");
            // обработка файла
            if (!workWithOneFile(patfFile)) {
                cout << "File is bad! Check file, please!" << endl;
                continue;
            }

        }
    }
}

// Метод для обработки одного файла.
bool Server::workWithOneFile(QString patfFile)
{
    auto dataFromFile = parsingXml(patfFile); // парсинг файла
    dataFromFile.pop_front(); // удаление корневого элемента <net>
    if (dataFromFile.size() == 0){
        cout << "File '%1' is bad! Check file, please!" << endl;
        return false;
    }
    auto newDataWithId = createGoodData(dataFromFile); // добавление привязки дочерних элементов к родительским
    for (auto elem: newDataWithId) {
        // добавление данных файла
        auto result = db.runQuery(db.createQuery(elem.keys()[0], elem.value(elem.keys()[0])), "insert");
        if (result.size() == 1) {
            if (result[0].keys()[0] == "error") {
                return false;
            }
        }
    }
    return true;
}

// Метод парсинга .xml файла. Сохранение всех тэгов и его атрибутов.
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
            QMap<QString, QString> elem = parsingXmlBlock(xml.attributes());
            element[xml.name().toString()] = elem;
        }
        if (element.size() != 0)
            result.push_back(element);
    }
    return result;
}

// Метод парсинга одного тэга из файла .xml
QMap<QString, QString> Server::parsingXmlBlock(QXmlStreamAttributes attrib)
{
    QMap<QString, QString> elem;
    for (int i = 0; i < attrib.size(); i++){
        elem[attrib[i].name().toString()] = attrib[i].value().toString();
    }
    return elem;
}

// Метод для привязки дочерних компонентов xml к родительским для дальнейшего их заноса в базу.
QVector<QMap<QString, QMap<QString, QString>>> Server::createGoodData(QVector<QMap<QString, QMap<QString, QString>>> data)
{
    QVector<QMap<QString, QMap<QString, QString>>> result;
    QVector<QString> tags; // список тэгов для поиска
    QVector<QString> idsTags; // список id тэгов для привязки
    for (auto elem: data) {
        QMap<QString, QMap<QString, QString>> row = elem;
        QString tag = elem.keys()[0]; // тэг на текущем шаге
        QString id = elem.value(elem.keys()[0]).value("id"); // id на текущем шаге
        if (tags.size() == 0){
            // если это первый элемент, то кладем их тэг и id в поисковые структуры
            tags.push_back(tag);
            idsTags.push_back(id);
            result.push_back(row);
            continue;
        }
        int indexTag = tags.indexOf(tag); // проверяем наличие данного тэга в поисковых структурах
        if (indexTag != -1) {
            // если такой тэг уже есть, то удалем все тэги после аналогичного (включительно)
            // (верхний тэг будет родителем текущего)
            tags = tags.mid(0, indexTag);
            idsTags = idsTags.mid(0, indexTag);
        }
        QString tagUp = tags.last(); // последний тэг в поисковой структуре (родительский)
        QString idUp = idsTags.last();
        // добавление нового поля id_*название родительского тэга* (для привязки к родителю)
        auto newMap = elem.value(elem.keys()[0]);
        newMap[QString("id_")+tagUp] = idUp;
        QMap<QString, QMap<QString, QString>> newRow;
        newRow[tag] = newMap;
        tags.push_back(tag);
        idsTags.push_back(id);
        result.push_back(newRow); // добавление в результат
    }
    return result;
}

// Метод, получения хэша файла
QString Server::getHashFile(QString pathFile)
{
    QString hashStr = "";
    QFile f(pathFile);
    if (f.open(QFile::ReadOnly)) {
        // Применение хэш-алгоритма Sha1
        QCryptographicHash hash(QCryptographicHash::Sha1);
        if (hash.addData(&f)) {
            hashStr += QString(hash.result().toHex());
        }
    }
    return hashStr;
}

void Server::incomingConnection(qintptr socketDescriptor){
    qDebug() << "client77777777777" << socketDescriptor;
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
        QMap<QString, QVector<QMap<QString, QString>>> mapResponse;
        auto tables = QString(TABLES_FOR_SEARCH).split(",");
        QString tmpSelectQuery = SELECT_ALL;
        for (auto table: tables){
            QString selectQuery = tmpSelectQuery.arg(table);
            auto resultBlock = db.runQuery(selectQuery, "select");
            mapResponse[table] = resultBlock;
        }
        SendToClient(mapResponse);
    }
    else
        qDebug() << "DataStream error";
}

void Server::onTimeout()
{
    // проверка работы БД
    if (!db.testRunBd()){
         cout << "Don't work DB!" << endl;
         return;
    }
    cout << "Start read files" << endl;
    QFileInfoList files = this->getFiles();
    if (files.size() == 0) {
        // выход в случае если файлов .xml нет или есть другие файлы в каталоге
        cout << "Error" << endl;
        return;
    }
    cout << "Finish read files" << endl;
    workWithFiles(files);
    cout << endl;
}

