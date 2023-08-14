#include <QCoreApplication>
#include "server.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server s("C:\\Users\\Admin\\Documents\\Qt\\server\\test", "C:\\Users\\Admin\\Documents\\Qt\\server\\database.db3");
    s.runServer();
    cout << "ГЫ" << endl;
    return a.exec();
}
