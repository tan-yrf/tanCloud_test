/*
 * @description: 客户端的中间件，主要负责界面、数据处理、数据传输部分的通信。
 *
 * @todo: 1.实例化客户端中所需的类对象。
 *
 *        2.连接多个信号与槽函数，实现组件间的通信。
 *
 *        3.实现界面间的跳转。
 *
 * @date: 2023/11/02
*/
#ifndef MYTCPCLIENT_H
#define MYTCPCLIENT_H

#include "datatransfer.h"
#include"login.h"
#include "clientwindow.h"

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>

class MyTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpClient();
    ~MyTcpClient();

private:    //私有变量
    Login *login;                                                   //登录注册页面
    ClientWindow *clientWindow;                                     //客户端主页面
    DataTransfer *dataTransfer;                                     //数据传输部分
private:    //私有函数
    void createDownloadDir();                                       //创建默认下载文件夹
    void connectLoginWithDataTransfer();                            //绑定登录注册页面和数据传输部分的信号与槽
    void connectSignalWithClient();                                 //绑定客户端接收的信号
    void connectMyFileWithDataTransfer();                           //绑定MyFile界面与数据传输部分的信号与槽
    void connectClientWindowWithDataTransfer();                     //绑定主界面与数据传输部分的信号与槽
signals:
    void fileListReq();

public slots:
    void onLoginSuccess();                                          //登录成功后关闭登录注册页面并跳转到主页面
};

#endif // MYTCPCLIENT_H
