#include "mytcpclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyTcpClient myTcpClient ;
    return a.exec();
}
