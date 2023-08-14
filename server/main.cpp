#include <QCoreApplication>
#include <iostream>

#include "server.h"
#include "cfg.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server s(PATH_DIR_WITH_XML, PATH_DB);
    s.runServer();
    cout << "ГЫ" << endl;
    return a.exec();
}
