/*
 * @description: 实现用户文件界面相关功能。
 *
 * @todo: 1. 界面显示和用户交互：
 *          updateMyFile()： 更新界面显示的当前文件夹路径。
 *          convertPath(QString filePath)： 将路径标准化，确保路径的格式正确。
 *          onTableRowClicked(const QModelIndex &index)： 处理用户点击表格行的事件，切换到点击的文件夹。
 *          on_btnUpload_clicked()： 处理用户点击上传按钮的事件，弹出文件选择对话框，获取用户选择的文件或文件夹路径。
 *          on_btnDownload_clicked()： 处理用户点击下载按钮的事件，获取用户选中的文件或文件夹，加入下载队列。
 *          on_btnNewDir_clicked()： 处理用户点击新建文件夹按钮的事件，弹出输入框，获取用户输入的文件夹名，发送创建文件夹信号。
 *          on_btnDelete_clicked()： 处理用户点击删除按钮的事件，获取用户选中的文件或文件夹，加入删除队列，发送删除文件信号。
 *          on_btnBack_clicked()： 处理用户点击返回按钮的事件，返回上一级文件夹。
 *          on_btnRefresh_clicked()： 处理用户点击刷新按钮的事件，请求刷新文件列表。
 *          on_btnSort_clicked() 和 on_btnDisplay_clicked()：处理用户点击排序和显示按钮的事件（未实现）。
 *
 *        2. 文件和文件夹操作：
 *          uploadFolder()： 上传文件夹，将文件夹队列中的文件夹上传到服务器。
 *          onUploadFileQueue()： 上传文件队列中的文件，将文件队列中的文件上传到服务器。
 *          downloadFolder()： 下载文件夹，将文件夹队列中的文件夹从服务器下载到本地。
 *          onDownloadFileQueue()： 下载文件队列中的文件，将文件队列中的文件从服务器下载到本地。
 *          createDir(QString folderPath)： 在本地创建文件夹。
 *          convertJsonArrayToVector(const QJsonArray &filesArray)： 将服务器返回的 JSON 数组转换为数据模型的数据。
 *          onDeleteFile()： 删除选中的文件或文件夹，将删除队列中的文件或文件夹从服务器删除。
 *
 *        3. 与数据传输组件的交互：
 *          onResultFromDataTransfer(qint8 resultType)： 处理数据传输组件返回的服务器处理结果，根据结果类型执行相应操作。
 *
 * @date: 2023/11/02
*/
#ifndef MYFILE_H
#define MYFILE_H

#include "constant.h"
#include "mydatamodel.h"

#include <QWidget>
#include <QDir>
#include <QQueue>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardItemModel>

namespace Ui {
class MyFile;
}

class MyFile : public QWidget
{
    Q_OBJECT

public:
    explicit MyFile(QWidget *parent = nullptr);
    ~MyFile();

signals:
    void uploadFileInfo(FileInfo);
    void sendChoiceToDataTransfer(qint8);                           //将用户选择发送给数据传输组件
    void downloadFileReq(QString);
    void fileListReq();                                             //文件列表请求
    void fileListDisplay();                                         //显示文件列表
    void createFolderReq(QString, QString);                         //创建文件夹信号
    void deleteFileReq(QString);
private :
    QVector<QStringList> convertJsonArrayToVector(const QJsonArray &array);
    void uploadFolder();
    void downloadFolder();
    void updateMyFile();
    void createDir(QString);

    QString convertPath(QString);                                   //路径标准化
public slots:
    void onResultFromDataTransfer(qint8);                           //接收数据传输组件返回的服务器处理结果

    void onFileListDisplay();                                       //显示文件列

    void onUploadFileQueue();                                       //上传文件队列中的文件

    void onDownloadFileQueue();                                     //下载文件队列中的文件

    void onTableRowClicked(const QModelIndex &index);

    void onDeleteFile();

    void on_btnUpload_clicked();

    void on_btnDownload_clicked();

    void on_btnNewDir_clicked();

    void on_btnShare_clicked();

    void on_btnDelete_clicked();

    void on_btnBack_clicked();

    void on_btnRefresh_clicked();

    void on_btnSort_clicked();

    void on_btnDisplay_clicked();

private:
    Ui::MyFile *ui;
    QDir* dir;
    MyDataModel* dataModel;

    QString currentPath;                            //用户处在文件空间中的位置，默认为根文件夹下
    QString fileListPath;
    //QString fileSavePath;
    QString downFolder;
    QString downloadPath;

    QQueue<FileInfo> upFolderQueue;                     //文件夹上传队列
    QQueue<FileInfo> upFileQueue;                       //文件上传队列

    QQueue<QString> downFolderQueue;                    //文件夹下载队列
    QQueue<QString> downFileQueue;                      //文件下载队列

    QQueue<QString> deleteQueue;                        //文件删除队列
};
#endif // MYFILE_H
