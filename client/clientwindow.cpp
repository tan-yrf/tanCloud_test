#include "clientwindow.h"
#include "./ui_clientwindow.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QTime>
#include <QCoreApplication>
#include <QDebug>

ClientWindow::ClientWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    topButton = new QButtonGroup();
    mySpace = new MySpace();

    //添加界面
    ui->stackedWidget->addWidget(mySpace);
    //添加按钮并设置界面值
    topButton->addButton(ui->btnMySpace, 0);

    //关联按钮
    connect(topButton, SIGNAL(buttonClicked(int)),
            ui->stackedWidget, SLOT(setCurrentIndex(int)));
}

ClientWindow::~ClientWindow()
{
    delete ui;
    delete topButton;
    delete mySpace;
}


/*实现延时功能,msec是毫秒数，比如要延时1秒,msec的值要设置为1000*/
void ClientWindow::sleep(qint32 msec)
{
    QTime targetTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < targetTime ){
        //得程序在while等待期间，去处理一下本线程的事件循环，处理事件循环最多100ms必须返回本语句，如果提前处理完毕，则立即返回这条语句。
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

/*关闭事件：用户退出客户端，发送信号给服务器结束线程*/
void ClientWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    //暂时没有进行逻辑判断，强行结束
    button = QMessageBox::question(this, tr("退出程序"), QString(tr("确认退出客户端?")));
    if(button == QMessageBox::No){
        event->ignore();
    }else if(button == QMessageBox::Yes){
        emit closeConnectServer();
        sleep(300);
        event->accept();  //接受退出信号，程序退出
        qDebug() <<"exit";
    }
}
