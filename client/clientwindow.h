#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTreeWidget>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::ClientWindow *ui;
    QTcpSocket *socket;
    QByteArray Data;
    bool connectSuccess;
    void createTree(QMap<QString, QVector<QMap<QString, QString>>> data);
    QMap<QString, QVector<QString>> getHeaders(QMap<QString, QVector<QMap<QString, QString>>> data);
    void SendToServer(QString str);
    void createOneRow(QTreeWidgetItem *parent,
                      QMap<QString, QVector<QString>> headers,
                      QMap<QString, QVector<QMap<QString, QString>>> allData,
                      QString idParentElem,
                      QString nameTableParent,
                      QVector <QString> nameTables,
                      bool isHeader);
    void checkCon();
public slots:
    void slotReadyRead();
    void slotDeleteCon();
};
#endif // CLIENTWINDOW_H
