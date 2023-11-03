#include "login.h"
#include "ui_login.h"
#include "constant.h"

#include <QDebug>
#include <QMessageBox>
#include <QNetworkAddressEntry>

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    isConnectedServer = false;        //初始化，未连接到服务器
}

Login::~Login()
{
    delete ui;
}


/*正则表达式判断输入的字符串是否为ipv4地址*/
bool Login::isIpv4(QString ip)
{
    QRegExp rxp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    if(!rxp.exactMatch(ip)){
        return false;
    }
    return true;
}


/*实现客户端连接到服务器事件，
 * 一个客户端只能连接一个服务器，连接成功后改变按钮提示信息，
 * 再次点击按钮取消与当前服务器的连接
 * 如果在登录注册界面连接到其他服务器需要先取消与当前服务器的连接*/
void Login::on_btnConnectServer_clicked()
{
    if(ui->btnConnectServer->text() == "断开连接"){
        emit closeConnectServer();                      //发送关闭连接信号给客户端
        ui->btnConnectServer->setText(tr("连接服务器"));
        return;
    }
    QString serverAddress = ui->editServerAddress->text();
    if(!isIpv4(serverAddress) || serverAddress.isNull()){  //无效ip地址
        QMessageBox::about(this, tr("注册信息错误"), tr("这是一个无效的服务器地址！"));
        qDebug() <<"无效ip地址，重新输入";
    }else{              //连接到服务器
        emit connectServer(serverAddress);
    }
}


/*在连接到服务器后且输入的账户名合法的前提下，将两次输入的密码进行比对，
 * 如果一致就将账号密码传递给客户端再转发给服务器*/
void Login::on_btnRegister_clicked()
{
    QString username = ui->editUsername->text();
    QString password = ui->editPassword->text();
    QString repassword = ui->editSurePassword->text();
    if(!isConnectedServer){
        QMessageBox::about(this, tr("注册信息错误"), tr("请先连接服务器！"));
        qDebug() <<"未连接到服务器";
    }else{
        if(username == NULL || password == NULL || repassword == NULL){
            QMessageBox::about(this, tr("注册信息错误"), tr("账号密码都不能为空"));
            qDebug() <<"账号密码都不能为空";
        }else if(password != repassword){
            QMessageBox::about(this, tr("注册信息错误"), tr("两次输入的密码不一致!"));
            qDebug() <<"两次输入的密码不一致";
        }else{
            emit sendRequestToClient(REGISTER_REQ, username, password);
            //qDebug() << username <<password;
        }
    }
}


/*在已连接服务器且输入的账号密码合法，两次输入的密码一致的前提下。
将账号密码发送给客户端，客户端发送到服务器进行验证。
如果登录成功就跳转到主页面，关闭登录注册页面*/
void Login::on_btnLogin_clicked()
{
    QString username = ui->editUsername->text();
    QString password = ui->editPassword->text();
    if(!isConnectedServer){
        QMessageBox::about(this, tr("登录信息错误"), tr("请先连接服务器！"));
        qDebug() <<"未连接到服务器";
    }else{
        if(username == NULL || password == NULL){
            QMessageBox::about(this, tr("登录信息错误"), tr("账号密码都不能为空"));
            qDebug() <<"账号密码都不能为空";
        }else{
            emit sendRequestToClient(LOGIN_REQ, username, password);
        }
    }
}


/*根据从服务器传递的连接服务器状态结果进行处理*/
void Login::onSocketStateFromClient(bool state)
{
    isConnectedServer = state;
    qDebug() << state;
    if(state == true){
        QMessageBox::about(this, tr("连接服务器"), tr("连接成功!"));
        ui->btnConnectServer->setText(tr("断开连接"));
    }else{
        QMessageBox::about(this, tr("连接服务器"), tr("连接失败!"));
    }
}


/*根据客户端返回的请求结果进行响应*/
void Login::onResultFromClient(qint8 resultType)
{
    switch (resultType) {
    case USER_EXISTS:
        //注册时用户名已经存在
        QMessageBox::about(this, tr("注册信息错误"), tr("用户名已存在"));
        break;
    case REGISTER_SUCCESS:
        //注册成功
        QMessageBox::about(this, tr("注册成功"), tr("注册成功，请登录账号"));
        break;
    case USERNAME_ERROR:
        //登陆时用户名错误
        QMessageBox::about(this, tr("登录失败"), tr("用户名错误"));
        break;
    case PASSWORD_ERROR:
        //登陆时密码错误
        QMessageBox::about(this, tr("登录失败"), tr("密码错误"));
        break;
    case LOGIN_FAILED:
        //未知原因登录失败
        QMessageBox::about(this, tr("登录失败"), tr("未知原因"));
        break;
    case LOGIN_SUCCESS:
        //登录成功
        QMessageBox::about(this, tr("登录成功"), tr("登录成功"));
        //发送信号跳转到主页面，并关闭登录注册页面
        emit loginSuccess();
        break;
    default:
        break;
    }
}

