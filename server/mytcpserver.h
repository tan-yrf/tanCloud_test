/*
 * @description: 服务器主线程类，为客户端连接创建新的线程。
 *
 * @todo: 1.监听端口：
 *          MyTcpServer()： 在构造函数中，服务器通过listen()函数开始监听特定的IP地址和端口号（这里监听了地址getIpAddress()返回的IP地址和端口号52996）。
 *
 *        2.连接处理：
 *          incomingConnection(qintptr socketDescriptor)： 当有新的连接请求时，服务器会调用这个函数为每个新的连接创建一个独立的线程（MyTcpSocketThread的实例），
 *          并将连接的唯一标识符（socketDescriptor）传递给新建的线程。线程负责处理与客户端的通信。
 *
 *        3.IP地址获取：
 *          getIpAddress()： 获取本机的IPv4地址，遍历网络接口，找到合适的网络接口并获取其IPv4地址。如果找不到合适的网络接口，则返回localhost地址。
 *
 *        4.创建用户文件夹：
 *          createUserDir()： 检查服务器运行时是否存在User和FileList文件夹，如果不存在则创建这两个文件夹。这两个文件夹用于存储用户的个人文件和文件列表信息。
 *
 * @date: 2023/11/02
*/
#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include "mytcpsocketthread.h"

#include <QObject>

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();
private:
    virtual void incomingConnection(qintptr socketDescriptor);  //为每一个新到来的连接建立一个线程
    QString getIpAddress();
    void createUserDir();                                       //服务器运行的时候如果没有User文件夹就会创建一个
signals:
};

#endif // MYTCPSERVER_H
