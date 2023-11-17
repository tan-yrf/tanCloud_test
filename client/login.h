/*
 * @description: 实现登录与注册功能的类
 * @todo: 1. 初始化和界面操作：
 *          explicit Login(QWidget *parent = nullptr)： 构造函数，初始化登录界面。
 *          bool isIpv4(QString ip)： 判断字符串是否为IPv4地址，用于输入合法性验证。
 *          void on_btnConnectServer_clicked()： 处理连接/断开服务器按钮点击事件，根据连接状态切换按钮功能，实现连接和断开服务器。
 *        2. 用户请求处理：
 *          void on_btnRegister_clicked()： 处理注册按钮点击事件，在连接服务器的前提下，验证输入合法性，发送注册请求给客户端。
 *          void on_btnLogin_clicked()： 处理登录按钮点击事件，在连接服务器的前提下，验证输入合法性，发送登录请求给客户端。
 *        3. 服务器连接和状态处理：
 *          void onSocketStateFromClient(bool)： 处理从客户端获取的连接状态，更新连接状态，弹出连接成功或失败的提示框。
 *        4. 请求结果处理：
 *          void onResultFromClient(qint8)： 处理客户端返回的请求结果，根据不同的结果类型弹出相应的提示框，如用户名已存在、注册成功、登录失败等。
 *
 * @date: 2023/11/2
*/
#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
    qint8 requestType;                                  //请求类型，登录或者注册
private:    //私有变量
    Ui::Login *ui;
    bool isConnectedServer;                             //判断是否连接到服务器
    QTimer timer;                                        //定时器
private:    //私有函数
    bool isIpv4(QString);                               //判断字符串是否为ipv4地址，返回结果
signals:
    void connectServer(QString);                        //连接服务器的信号
    void closeConnectServer();                          //断开与服务器的连接信号
    void sendRequestToClient(qint8, QString, QString);  //发送请求给客户端
    void loginSuccess();                                //登录成功，跳转主页面
private slots:
    void onTimeout();                                   //超时就断开与服务器的连接
public slots:
    void on_btnConnectServer_clicked();                 //实现连接服务器
    void on_btnRegister_clicked();                      //实现发送注册请求
    void on_btnLogin_clicked();                         //实现发送登录请求
    void onSocketStateFromClient(bool);                 //根据从客户端获取的连接状态做出响应
    void onResultFromClient(qint8);                     //根据客户端返回的请求结果进行处理
};

#endif // LOGIN_H
