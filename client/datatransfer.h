/*
 * @description: 负责与服务器进行通信的类。
 *
 * @todo: 1. 初始化和线程运行：
 *          init()： 初始化各种成员变量，建立服务器与客户端的连接。
 *          onConnectServer(QString address)： 建立服务器与客户端的连接，处理服务器连接请求。
 *          onCloseConnectServer()： 关闭服务器连接，断开与服务器的连接。
 *          onSendRequestToServer(qint8 requestType, QString str1, QString str2)： 向服务器发送请求信息。
 *
 *        2. 客户端通信：
 *          onReadMessage()： 从服务器接收请求信息。
 *          handleResultFromServer(qint8 resultType)： 处理从服务器返回的处理结果。
 *          onSendFileToServer(qint64 numBytes)： 将文件数据发送给服务器。
 *
 *        3. 客户端请求处理：
 *          onUploadFile(FileInfo fileInformation)： 处理客户端上传文件请求。
 *          onDownloadFile(QString targetPath)： 处理客户端下载文件请求。
 *          onCreateFolder(QString targetPath, QString folderName)： 处理客户端创建文件夹请求。
 *          onDeleteFileReq(QString targetPath)： 处理客户端删除文件请求。
 *          onFileListReq()： 处理客户端文件列表请求。
 *
 *        4. 文件上传模块：
 *          sendFileInfoToServer()： 将待上传的文件的信息发送给服务器。
 *          onSendFileToServer(qint64 numBytes)： 将文件数据发送给服务器。
 *
 *        5. 文件下载模块：
 *          getFileFromServer()： 处理从服务器接收文件的逻辑。
 *          getFileHead()： 接收文件头信息。
 *          getFileContent()： 接收文件内容。
 *
 * @date: 2023/11/2
*/
#ifndef DATATRANSFER_H
#define DATATRANSFER_H

#include "constant.h"
#include <QObject>
#include <QFile>
#include <QDir>
#include <QTcpSocket>
#include <QAbstractSocket>

class DataTransfer : public QObject
{
    Q_OBJECT
public:
    explicit DataTransfer(QObject *parent = nullptr);
    ~DataTransfer();

private:    //私有变量
    QTcpSocket *tcpSocket;                                  //实现与服务器通信的套接字
    qint64 messageSize;                                     //接收到的信息大小
    QString downloadFolder;                                 //默认下载文件所在文件夹
    QString downloadPath;                                   //逻辑上的下载路径
    QString fileListPath;                                   //fileList文件
    bool isFileList;                                        //标记接收的文件是否为.fileList文件
    //上传文件部分
    FileInfo upFileInfo;                                    //待上传的文件的文件信息
    QFile* fileToUpload;                                    //用来打开待上传的文件
    qint64 upTotalBytes;                                    //上传的文件总大小
    qint64 upBytesWritten;                                  //已上传大小
    qint64 upBytesToWrite;                                  //要上传的大小
    bool isUploadingFile;                                   //上传标记
    QByteArray  upBlock;                                    //上传缓冲区
    //下载文件部分
    FileInfo downFileInfo;                                  //待下载的文件的文件信息
    QFile* fileToDownload;                                  //用来储存待下载的文件
    qint64 downTotalBytes;                                  //下载的文件总大小
    qint64 downFileInfoSize;                                //待下载文件的文件基本信息大小
    qint64 downBytesReceived;                               //已下载大小
    qint64 downBytesToReceive;                              //要下载的大小
    bool isDownloadingFile;                                 //下载状态
    QByteArray  downBlock;                                  //下载缓冲区
private:    //私有函数
    void init();                                            //变量初始化
    void sleep(qint32 msec);                                //延时函数,msec是需要延时的毫秒数
    void getResultFromServer();                             //接收从服务器返回的请求结果
    void sendFileInfoToServer();                            //将文件信息上传给服务器
    void handleResultFromServer(qint8);                     //根据服务器返回的结果进行处理
    void getFileFromServer();                               //接收服务器传输的文件
    bool getFileHead();
    void getFileContent();

signals:
    void sendSocketStateToLogin(bool);                      //将连接结果传递到登录注册页面
    void sendResultToLogin(qint8);                          //将请求结果返回给登录注册页面
    void sendRequestToServer(qint8, QString, QString);      //发送请求给服务器
    void sendResultToMyFile(qint8);                         //把服务器返回的文件上传请求结果发送给MyFile界面
    void fileListDisplayToMyFile();
    void uploadComplete();                                  //文件上传完成信号
    void downloadComplete();                                //文件下载完成信号
    void deleteFileComplete();                              //文件删除完成
public slots:
    void onConnectServer(QString);                          //响应登录界面的连接服务器信号，与输入的地址的服务器连接
    void onCloseConnectServer();                            //响应关闭服务器信号
    void onSendRequestToServer(qint8, QString, QString);    //向服务器发送请求
    void onReadMessage();                                   //每当客户端接收信息就进行读取
    void onUploadFile(FileInfo);                            //接受上传文件的请求并进行处理
    void onCreateFolder(QString, QString);                  //接收MyFile界面创建文件夹请求
    void onChoiceFromMyFile(qint8);                         //接收MyFIle界面的用户选择信息s
    void onSendFileToServer(qint64);                        //发送文件给服务器
    void onDownloadFile(QString);                           //接收MyFile界面下载文件的请求并进行处理
    void onFileListReq();                                   //请求服务器发送文件列表
    void onDeleteFileReq(QString);                          //请求服务器删除目标路径下的文件
};

#endif // DATATRANSFER_H
