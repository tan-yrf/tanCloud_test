#include "serverwindow.h"
#include "mytcpserver.h"


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //创建一个MyTcpServer对象处理连接问题，不知道这样做有没有什么问题以及什么时候关闭服务器，先存疑
    MyTcpServer *myTcpServer = new MyTcpServer();
    return a.exec();
}
