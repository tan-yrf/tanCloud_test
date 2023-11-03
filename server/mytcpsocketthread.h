/*
 * @description: 实现服务器与客户端通信，对客户端的请求进行处理。
 *
 * @todo: 1.初始化和线程运行：
 *          init()： 初始化数据库操作对象、线程状态标志等各种成员变量，并建立服务器与客户端的连接。
 *          run()： 线程主函数，处理客户端请求和文件传输。
 *
 *        2.客户端通信：
 *          getRequestFromClient()： 从客户端接收请求信息。
 *          sendResultToClient(qint8 messageType)： 向客户端发送处理结果信息。
 *          handleRequest(qint8 requestType, QString str1, QString str2)： 根据不同的请求类型进行相应的处理，包括登录、注册、上传文件、下载文件、获取文件列表等操作。
 *
 *        3.客户端请求处理：
 *          logoutExecution()： 处理用户退出登录请求，关闭连接和线程。
 *          registerExecution(QString username, QString password)： 处理用户注册请求，调用数据库处理注册逻辑。
 *          loginExecution(QString username, QString password)： 处理用户登录请求，调用数据库处理登录逻辑，验证用户身份。
 *          handleFileListReq()： 处理客户端文件列表请求，获取用户文件列表信息并发送给客户端。
 *          createFolder(QString targetPath, QString folderName)： 处理客户端创建文件夹请求，创建指定路径下的文件夹。
 *          deleteFile(QString targetPath)： 处理客户端删除文件或文件夹请求，根据目标路径删除对应的文件或文件夹。
 *
 *        4.文件上传模块：
 *          getUploadFileRequest(QString fileSize, QString targetPath)： 处理客户端上传文件请求，判断文件是否存在、是否可以上传等。
 *          getFileFromClient()： 处理从客户端接收文件的逻辑，包括接收文件头和文件内容。
 *          getFileHead()： 接收文件头信息。
 *          getFileContent()： 接收文件内容。
 *
 *        5.文件下载模块：
 *          getDownloadFileRequest(QString targetPath)： 处理客户端下载文件请求，判断文件是否存在、获取文件信息等。
 *          sendFileInfoToClient()： 将文件信息发送给客户端。
 *          onSendFileToClient(qint64 numBytes)： 将文件发送给客户端，实现文件传输的功能。
 *
 * @date: 2023/11/02
*/
#ifndef MYTCPSOCKETTHREAD_H
#define MYTCPSOCKETTHREAD_H

#include <QtNetwork>
#include "dbhandle.h"
#include "constant.h"

class MyTcpSocketThread : public QThread
{
    Q_OBJECT
public:
    MyTcpSocketThread();
    ~MyTcpSocketThread();
    qintptr ptr;            //socket唯一标识符
    QTcpSocket *socket;
    void writeSocketDescriptor(qintptr socketDescriptor){
        ptr = socketDescriptor;
    }
private:    //私有变量
    DBHandle *database;                             //进行数据库操作
    bool threadRunning;                             //线程是否开始运行
    bool isLogin;                                   //为真表示用户登录成功
    QString userName;
    QString userDir;                                //每个用户对应的文件夹
    QString fileListPath;                           //文件列表路径

    //与客户端上传文件相关
    bool isGettingFile;                             //是否处于接收文件状态
    qint64 upTotalBytes;
    qint64 upBytesReceived;
    qint64 upFileInfoSize;
    QFile* upFile;
    FileInfo upFileInfo;
    QByteArray upBlock;

    //与客户端下载文件相关
    bool isDownloadingFile;                         //是否处于下载文件状态,下载是对于客户端来说的
    qint64 downTotalBytes;
    qint64 downBytesWritten;
    qint64 downBytesToWrite;
    qint64 downFileInfoSize;
    QFile* downFile;
    FileInfo downFileInfo;
    QByteArray downBlock;

private:    //私有函数
    void init();                                    //变量初始化
    virtual void run();                             //线程运行

    void sendResultToClient(qint8);                 //向客户端发送信息
    void getRequestFromClient();                    //从客户端接收信息
    void handleRequest(qint8, QString, QString);    //处理客户端请求
    void logoutExecution();                         //用户退出登录
    void registerExecution(QString, QString);       //注册新用户
    void loginExecution(QString, QString);          //用户登录
    void getUploadFileRequest(QString, QString);    //用户请求上传文件
    void getFileFromClient();                       //接收用户上传的文件
    bool getFileHead();                             //接收文件头
    void getFileContent();                          //接收文件内容
    void getDownloadFileRequest(QString);           //用户请求下载文件
    void sendFileInfoToClient();                    //将文件信息发送给客户端
    void onSendFileToClient(qint64);                //传输文件给客户端
    void saveFileList();                            //保存文件列表
    void handleFileListReq();                       //处理客户端文件列表请求
    void createFolder(QString, QString);            //创建文件夹
    void deleteFile(QString);                       //删除文件
    QString convertPath(QString);                   //将路径转换为标准路径
};

#endif // MYTCPSOCKETTHREAD_H
