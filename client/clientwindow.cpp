#include "clientwindow.h"
#include "./ui_clientwindow.h"

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

