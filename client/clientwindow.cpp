#include "clientwindow.h"
#include "ui_clientwindow.h"



ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ClientWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ClientWindow::slotDeleteCon);
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

// Слот клика на кнопку подключения для установления прослушивания сервера
void ClientWindow::on_pushButton_clicked()
{
    socket->connectToHost("127.0.0.1", 2323);
    ui->pushButton->setStyleSheet("background-color: green");
}

//Слот для удаления подключения
void ClientWindow::slotDeleteCon() {
    socket->deleteLater();
    ui->pushButton->setStyleSheet("background-color: red");
}
// Слот считывания ответа от сервера и построения дерева
void ClientWindow::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_14);
    if (in.status() == QDataStream::Ok){
        QMap<QString, QVector<QMap<QString, QString>>> vector;
        in >> vector;
        if (vector["block"].size() == 0) {
            // Случай когда данные пришли пустые
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, "No data available! Check database, please.");
            item->setBackground(0, Qt::red);
            item->setForeground(0, Qt::yellow);
            ui->treeWidget->setColumnCount(1);
            ui->treeWidget->addTopLevelItem(item); // установка заголовка верхнего уровня
            return;
        }
        createTree(vector); // построение дерева
    }
}

void ClientWindow::on_pushButton_2_clicked()
{
    SendToServer("all");
}

// Рекурсивный метод построения одного элемента дерева.
// Параметры: parent - родительский элемент
//            headers - заголовки для каждой из таблиц
//            allData - все данные из таблиц
//            idParentElem - id родительского элемента
//            nameTableParent - название родительской таблицы
//            nameTables - вектор с названиями таблиц для дальнейшего обхода (когда пустой стоп Рекурсия)
//            isHeader - наличие заголовка
void ClientWindow::createOneRow(QTreeWidgetItem *parent,
                  QMap<QString, QVector<QString>> headers,
                  QMap<QString, QVector<QMap<QString, QString>>> allData,
                  QString idParentElem,
                  QString nameTableParent,
                  QVector <QString> nameTables,
                  bool isHeader){
    bool insertHeader = isHeader ? false : true; // параметр, который говорит нужно ли вставялять заголовок
    QTreeWidgetItem *headerChildren = new QTreeWidgetItem(); // элемент на нужном шаге
    QString nameTableNow = nameTables[0];
    auto nameTablesNow = nameTables; // создание нового списка чтобы не удалялось на последующих шагах
    nameTablesNow.pop_front(); // удаление текущей таблицы
    auto dataChildren = allData[nameTableNow];
    for (auto tableChild: dataChildren){ // обход таблицы текущей
        QString idNowElement = tableChild["id"];
        if (idParentElem != "" && tableChild[QString("id_%1").arg(nameTableParent)] != idParentElem){
            continue;
        }
        if (insertHeader) {
            // вставка заголовка
            int countColumns = headers[nameTableNow].size();
            for (int j = 0; j < countColumns; j++) {
                headerChildren->setText(j, headers[nameTableNow][j]);
                headerChildren->setBackground(j, Qt::yellow);
                headerChildren->setForeground(j, Qt::red);
            }
            parent->addChild(headerChildren);
            insertHeader = false;
        }
        QTreeWidgetItem *child = new QTreeWidgetItem();
        for (int j = 0; j < tableChild.size(); j++) {
            // установка текста
            child->setText(j, tableChild[tableChild.keys()[j]]);
        }
        if (isHeader) {
            parent->addChild(child);
        }
        else {
            headerChildren->addChild(child);
        }
        if (nameTablesNow.size() != 0){
            // рекурсивный вызов для дальнейших элементов
            createOneRow(child, headers, allData, idNowElement, nameTableNow, nameTablesNow, false);
        }
    }
}

// Метод построения дерева элементов
void ClientWindow::createTree(QMap<QString, QVector<QMap<QString, QString>>> data)
{
    ui->treeWidget->clear();
    QMap<QString, QVector<QString>> headers = getHeaders(data); // получение заголовков таблиц
    int maxColumns = 0;
    for (auto header: headers.keys()) {
        maxColumns = headers[header].size() > maxColumns ? headers[header].size() : maxColumns;
    }
    ui->treeWidget->setColumnCount(maxColumns); // Установка максимального числа столбцов
    // Создание вектора последовательности таблиц
    QVector<QString> treeNames;
    treeNames.push_back("block");
    treeNames.push_back("board");
    treeNames.push_back("port");
    auto tableName = treeNames[0];
    // начальный элемент
    QTreeWidgetItem *item = new QTreeWidgetItem();
    int countColumns = headers[tableName].size();
    for (int j = 0; j < countColumns; j++) {
        item->setText(j, headers[tableName][j]);
        item->setBackground(j, Qt::yellow);
        item->setForeground(j, Qt::red);

    }
    ui->treeWidget->addTopLevelItem(item); // установка заголовка верхнего уровня
    // Вызов рекурсивного обхода
    createOneRow(item, headers, data, "", "", treeNames, true);
}

// Метод для получения заголовков из входных данных
QMap<QString, QVector<QString>> ClientWindow::getHeaders(QMap<QString, QVector<QMap<QString, QString> > > data)
{
    QMap<QString, QVector<QString>> result;
    for (auto key: data.keys()){
        QVector<QString> header;
        if (data[key].size() == 0){
            result[key] = header;
            continue;
        }
        header = data[key][0].keys().toVector();
        result[key] = header;
    }
    return result;
}


void ClientWindow::SendToServer(QString str)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_14);
    out << str;
    socket->write(Data);
}
