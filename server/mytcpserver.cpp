
#include "mytcpserver.h"
#include <QList>
#include <QDebug>
#include <QtNetwork>
#include <QDir>
#include <QCoreApplication>

MyTcpServer::MyTcpServer()
{
    QString address = this->getIpAddress();
    //监听52996这个端口
    //QHostAddress address = QHostAddress(this->address);
    this->listen(QHostAddress(address), 52996);
    //this->listen(QHostAddress::LocalHost, 52996);
    createUserDir();
}


/*每当有一个新的连接就调用这个函数为连接新建一个线程，现在暂时不考虑连接数过多的情况*/
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug()<<"new connect"<<socketDescriptor;
    MyTcpSocketThread* pTcpSocket = new MyTcpSocketThread();

    pTcpSocket->writeSocketDescriptor(socketDescriptor);    //将这个连接socket唯一标识符赋值给新建立的MyTcpSocket对象
    pTcpSocket->moveToThread(pTcpSocket);
    pTcpSocket->start();            //开始运行线程

}


/*获取本机以太网接口的IPv4地址,如果没有则返回localhost
ps:暂时没有考虑使用其他网络接口，或者多网络接口的情况*/
QString MyTcpServer::getIpAddress()
{
    QString ip;
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();

    for(const QNetworkInterface &interface : interfaceList){
        // IsUp: 网络接口处于活动状态, IsRunning: 网络接口已分配资源, Ethernet: 类型为以太网接口
        if(interface.isValid() && interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            interface.type() == QNetworkInterface::Ethernet &&
            !interface.humanReadableName().contains("WSL", Qt::CaseInsensitive)){ // 排除描述信息中包含 "WSL" 的接口

            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for(const QNetworkAddressEntry &entry : entries){
                if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol){ // 获取IPv4地址
                    QString ipAddress = entry.ip().toString();
                    qDebug() << ipAddress;
                    return ipAddress;
                }
            }
        }
    }

    // 如果没有找到有效的IPv4地址，返回localhost
    qDebug() << QHostAddress(QHostAddress::LocalHost).toString();
    return QHostAddress(QHostAddress::LocalHost).toString();
}



/*服务器运行的时候没有User文件夹和fileList文件夹就会创建一个*/
void MyTcpServer::createUserDir()
{
    // 检查User文件夹是否存在，如果不存在就创建它
    QString appDir = QCoreApplication::applicationDirPath();
    QString nativeAppDir = QDir::toNativeSeparators(appDir);
    QString userFolderPath = QDir(nativeAppDir).filePath("User");
    QDir userDir(userFolderPath);
    if (!userDir.exists()) {
        if (userDir.mkpath(".")) {
            qDebug() << "User folder created successfully.";
        } else {
            qDebug() << "Failed to create User folder.";
        }
    } else {
        //qDebug() << "User folder already exists.";
    }

    QString fileListFolderPath = QDir(nativeAppDir).filePath("FileList");
    QDir fileListDir(fileListFolderPath);
    if(!fileListDir.exists()){
        if (fileListDir.mkpath(".")) {
            qDebug() << "FileList folder created successfully.";
        } else {
            qDebug() << "Failed to create FileList folder.";
        }
    }else{
        //qDebug() << "FileList folder already exists.";
    }

}
