/*
 *@description: 客户端的主界面
 *
 *@todo: 1.实例化所有界面。
 *
 *       2.实现顶层导航栏，根据用户选择的界面进行跳转。
 *
 *@date: 2023/11/2
*/
#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include "myspace.h"
#include <QWidget>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QWidget
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();
    MySpace* mySpace;
private:    //私有函数
    void sleep(qint32 msec);                                //延时函数,msec是需要延时的毫秒数
    void closeEvent(QCloseEvent *event);                    //关闭事件
signals:
    void closeConnectServer();
private:    //私有变量
    Ui::ClientWindow *ui;
    QButtonGroup* topButton;

};
#endif // CLIENTWINDOW_H
