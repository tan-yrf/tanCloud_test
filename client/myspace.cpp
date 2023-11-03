#include "myspace.h"
#include "ui_myspace.h"


MySpace::MySpace(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MySpace)
{
    ui->setupUi(this);
    subButton = new QButtonGroup();
    myFile = new MyFile();

    //添加界面
    ui->stackedWidget->addWidget(myFile);
    //添加按钮并设置界面值
    subButton->addButton(ui->btnMyFile, 0);

    //关联按钮
    connect(subButton, SIGNAL(buttonClicked(int)),
            ui->stackedWidget, SLOT(setCurrentIndex(int)));
}

MySpace::~MySpace()
{
    delete ui;
    delete subButton;
    delete myFile;
}
