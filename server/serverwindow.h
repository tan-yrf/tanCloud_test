#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QWidget
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:
    Ui::ServerWindow *ui;
};
#endif // SERVERWINDOW_H
