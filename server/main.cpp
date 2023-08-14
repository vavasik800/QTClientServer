#include <QCoreApplication>
#include "server.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server s("C:\\Users\\vasil\\Qt\\server\\test");
    s.runServer();
    cout << "ГЫ" << endl;
    return a.exec();
}
