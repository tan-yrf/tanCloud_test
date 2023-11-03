/*
 * @description: 实现用户空间界面的类
 *
 * @todo: 1、实现次级导航栏，在用户空间下的界面间进行跳转。
 *
 * @date: 2023/11/02
*/
#ifndef MYSPACE_H
#define MYSPACE_H

#include "myfile.h"
#include <QWidget>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui {
class MySpace;
}
QT_END_NAMESPACE

class MySpace : public QWidget
{
    Q_OBJECT

public:
    explicit MySpace(QWidget *parent = nullptr);
    ~MySpace();
    MyFile* myFile;
signals:
    void uploadfile(FileInfo);
private:
    Ui::MySpace *ui;
    QButtonGroup* subButton;

};

#endif // MYSPACE_H
