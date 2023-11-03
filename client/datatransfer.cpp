#include "datatransfer.h"

#include <QTime>
#include <QDataStream>
#include <QNetworkAddressEntry>
#include <QCoreApplication>

DataTransfer::DataTransfer(QObject *parent)
    : QObject{parent}
{

    init();
    //发送请求信息给服务器
    connect(this, &DataTransfer::sendRequestToServer, this, &DataTransfer::onSendRequestToServer);
}

DataTransfer::~DataTransfer()
{
    // 断开与服务器的连接
    tcpSocket->disconnectFromHost();
    tcpSocket->waitForDisconnected();

    // 关闭文件并释放资源
    if (fileToUpload) {
        fileToUpload->close();
        delete fileToUpload;
    }

    if (fileToDownload) {
        fileToDownload->close();
        delete fileToDownload;
    }

    // 删除动态分配的QTcpSocket对象
    if (tcpSocket) {
        tcpSocket->deleteLater();
    }
}

/*------------------------初始化模块--------------------------*/
/*变量初始化*/
void DataTransfer::init()
{
    tcpSocket = new QTcpSocket(this);
    messageSize = 0;
    QString appDir = QCoreApplication::applicationDirPath();
    QString nativeAppDir = QDir::toNativeSeparators(appDir);
    downloadFolder = QDir(nativeAppDir).filePath("download");
    downloadPath = "";
    isFileList = false;
    fileListPath = QDir(nativeAppDir).filePath(".fileList");

    // 上传文件部分初始化
    fileToUpload = nullptr;
    upTotalBytes = 0;
    upBytesWritten = 0;
    upBytesToWrite = 0;
    isUploadingFile = false;
    upBlock.clear();

    // 下载文件部分初始化
    fileToDownload = nullptr;
    downTotalBytes = 0;
    downFileInfoSize = 0;
    downBytesReceived = 0;
    downBytesToReceive = 0;
    isDownloadingFile = false;
    downBlock.clear();
}


/*实现延时功能,msec是毫秒数，比如要延时1秒,msec的值要设置为1000*/
void DataTransfer::sleep(qint32 msec)
{
    QTime targetTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < targetTime ){
        //得程序在while等待期间，去处理一下本线程的事件循环，处理事件循环最多100ms必须返回本语句，如果提前处理完毕，则立即返回这条语句。
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}


/*接收登录界面传递的服务器地址，测试能否与输入的服务器连接*/
void DataTransfer::onConnectServer(QString address)
{
    qDebug() << address;
    QHostAddress ip = QHostAddress(address);
    //连接服务器，我设置的服务器端口是52996
    tcpSocket->connectToHost(ip, 52996);
    //因为连接服务器要一定时间，所以做一个延时处理
    sleep(500);     //延时0.5s
    qDebug() <<ip <<tcpSocket->state();
    if(tcpSocket->state() == QAbstractSocket::ConnectedState){
        emit sendSocketStateToLogin(true);      //连接成功传递信号到登录注册界面

        //如果客户端处于传输文件中，每当缓冲区可以容纳新数据，就接收这个信号将文件的下一部分写入缓冲区
        connect(tcpSocket, &QTcpSocket::bytesWritten, this, &DataTransfer::onSendFileToServer);
        //每当有可读信息就用onReadMessage()函数进行读取
        connect(tcpSocket, &QTcpSocket::readyRead, this, &DataTransfer::onReadMessage);
    }else{
        emit sendSocketStateToLogin(false);
    }
}


/*断开与服务器的连接*/
void DataTransfer::onCloseConnectServer()
{
    tcpSocket->disconnectFromHost();
    qDebug() << "连接状态" << tcpSocket->state();
}
/*------------------------初始化模块结束--------------------------*/

/*------------------------处理请求模块--------------------------*/

/*发送请求信息给服务器，第一个参数为请求类型，服务器根据请求类型进行响应
 * 第二、三个参数是为了通用性，设置为在不同情况下传递的相关信息*/
void DataTransfer::onSendRequestToServer(qint8 requestType, QString str1, QString str2)
{
    QByteArray requestBlock;                                        //存放请求信息的二进制数据的缓冲区
    QDataStream out(&requestBlock, QIODevice::WriteOnly);           //用于将请求信息序列化(就是将请求信息转换成二进制信息)
    out.setVersion(QDataStream::Qt_5_15);                           //设置数据流版本
    //预先填入一个8字节的0，因为设计的协议前8个字节表示数据包大小
    out <<(qint64)0;
    out << requestType << str1 << str2;                             //输入实际数据
    out.device()->seek(0);                                          //跳转到数据头部
    out << (qint64)(requestBlock.size() - sizeof(qint64));          //实际数据的大小
    tcpSocket->write(requestBlock);                                 //发送数据
    qDebug() << "待发送的数据大小" << tcpSocket->bytesToWrite();
    tcpSocket->waitForBytesWritten();                               //等待数据传输完成
    if(tcpSocket->bytesToWrite() == 0) qDebug() <<"send over.  " << requestType << str1 << str2;
    requestBlock.resize(0);                                         //清空
}


/*接收信息函数，每当客户端有可读信息就进行读取，并对信息进行解析*/
void DataTransfer::onReadMessage()
{
    if(!isDownloadingFile){                     //接收服务器返回的请求结果，并对结果进行处理
        getResultFromServer();
    }else{                                      //接收服务器传输的文件
        getFileFromServer();
    }
}


/*接收服务器返回的请求结果，并根据请求结果进行处理*/
void DataTransfer::getResultFromServer()
{
    qDebug()<<"\n" <<"正在接收请求信息";
    qint8 resultType;
    messageSize = 0;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_15);
    if(messageSize == 0){   //如果是刚开始接收数据
        //判断接收的数据是否大于八字节，也就是数据的大小信息所占的空
        //如果是则保存到blockSize变量中，否则直接返回，继续接收数据
        if(tcpSocket->bytesAvailable() < (int)sizeof(qint64)) return;
        in >> messageSize;
    }
    //如果没有得到全部的数据，则返回，继续接收数据
    if(tcpSocket->bytesAvailable() < messageSize) return;
    //将接收到的数据存放在变量中
    in >> resultType;
    qDebug() << "receive:" << resultType;

    //根据返回的结果进行处理
    handleResultFromServer(resultType);
}


/*根据服务器返回的结果进行处理*/
void DataTransfer::handleResultFromServer(qint8 resultType)
{
    switch (resultType) {
    case USER_EXISTS:
        //注册时用户名已经存在
        emit sendResultToLogin(USER_EXISTS);
        break;
    case REGISTER_SUCCESS:
        //注册成功
        emit sendResultToLogin(REGISTER_SUCCESS);
        break;
    case USERNAME_ERROR:
        //登陆时用户名错误
        emit sendResultToLogin(USERNAME_ERROR);
        break;
    case PASSWORD_ERROR:
        //登陆时密码错误
        emit sendResultToLogin(PASSWORD_ERROR);
        break;
    case LOGIN_FAILED:
        //未知原因登录失败
        emit sendResultToLogin(LOGIN_FAILED);
        break;
    case LOGIN_SUCCESS:
        //登录成功,请求服务器发送文件列表
        emit sendResultToLogin(LOGIN_SUCCESS);
        //emit sendRequestToServer(FILE_LIST_REQ, "", "");
        break;
    case CAN_UPLOAD:
        //服务器返回结果，可以开始上传文件
        emit sendResultToMyFile(CAN_UPLOAD);
        break;
    case CAN_DOWNLOAD:
        //服务器返回结果，可以下载文件
        emit sendResultToMyFile(CAN_DOWNLOAD);
        break;
    case FILE_NOT_EXISTS:
        //服务器返回结果，文件不存在
        break;
    case FILE_LIST_INFO:
        //处理服务器传输的文件列表
        downTotalBytes = 0;
        downBytesReceived = 0;
        downBytesToReceive = 0;
        downFileInfoSize = 0;
        isDownloadingFile = true;     //修改正在下载文件标记
        isFileList = true;
        qDebug() <<"正下载文件";
        break;
    case FILE_TRANSFER_COMPLETE:
        //文件传输完成，传递信号给MyFile界面
        emit uploadComplete();
        break;
    case DELETE_FILE_COMPLETE:
        //文件删除完成，传递信号给MyFile界面
        emit deleteFileComplete();
        break;
    default:
        break;
    }
}


/*请求服务器发送文件列表*/
void DataTransfer::onFileListReq()
{
    emit sendRequestToServer(FILE_LIST_REQ, "", "");

}


/*接收MyFile发出的创建文件夹信号，发送待创建的文件夹的名称和目标路径*/
void DataTransfer::onCreateFolder(QString targetPath, QString folderName)
{
    qDebug() <<"文件夹名：" <<folderName;
    qDebug() <<"目标路径：" <<targetPath;
    emit sendRequestToServer(CREATE_FOLDER_REQ, targetPath, folderName);
}


/*接收MyFile发出的删除文件信号，发送待删除的文件路径给服务器*/
void DataTransfer::onDeleteFileReq(QString targetPath)
{
    qDebug() <<"删除文件路径";
    emit sendRequestToServer(DELETE_FILE_REQ, targetPath, "");
}


/*接收MyFile界面传递的用户选择信息*/
void DataTransfer::onChoiceFromMyFile(qint8 choice)
{
    switch (choice) {
    case SURE_UPLOAD:
    {
        emit sendRequestToServer(SURE_UPLOAD, "", "");
        upTotalBytes = 0;
        upBytesWritten = 0;
        upBytesToWrite = 0;
        sendFileInfoToServer();
        break;
    }
    case CANCEL_UPLOAD:
    {
        break;
    }
    case SURE_DOWNLOAD:
    {
        emit sendRequestToServer(SURE_DOWNLOAD, "", "");
        downTotalBytes = 0;
        downBytesReceived = 0;
        downBytesToReceive = 0;
        isDownloadingFile = true;     //修改正在下载文件标记
        downFileInfoSize = 0;

        qDebug() <<"正下载文件";
        break;
    }
    case CANCEL_DOWNLOAD:
    {
        break;
    }
    default:
        break;
    }

}
/*------------------------处理请求模块结束--------------------------*/

/*------------------------处理上传文件功能模块--------------------------*/
/*整体思路：当主界面发出上传文件信号时,onUpload函数接收信号。
* 1、发出上传文件请求到服务器，请求中包含文件大小和文件路径，由服务器判断是否能够上传。
* 2、服务器收到请求后进行响应返回结果。
* 3、服务器返回的结果表示能够进行上传，就将文件信息发送给服务器。
* 4、将文件内容发送给服务器。*/

/*接收MyFile发出的上传文件信号,然后发送文件上传路径和文件大小给服务器，让服务器判断是否能够上传*/
void DataTransfer::onUploadFile(FileInfo fileInformation)
{
    upFileInfo = fileInformation;

    upTotalBytes = upFileInfo.fileSize;              //获取文件大小
    upBytesToWrite = upFileInfo.fileSize;

    // 输出文件信息
    qDebug() << "文件名："   << upFileInfo.fileName;
    qDebug() << "文件大小：" << upFileInfo.fileSize << "字节";
    qDebug() << "文件类型：" << upFileInfo.fileType;
    qDebug() << "文件路径：" << upFileInfo.filePath;
    qDebug() << "目标路径：" << upFileInfo.targetPath;
    qDebug() << "上传时间：" << upFileInfo.uploadTime;
    emit sendRequestToServer(UPLOAD_REQ, QString::number(upTotalBytes), upFileInfo.targetPath);
}


/*在收到服务器返回的允许上传信号后确认上传文件的话，将文件信息上传给服务器*/
void DataTransfer::sendFileInfoToServer()
{
    fileToUpload = new QFile(upFileInfo.filePath);
    if(!fileToUpload->open(QIODevice::ReadOnly)){
        qDebug() << "无法打开文件：" << fileToUpload->errorString();
        return;
    }
    upTotalBytes = upFileInfo.fileSize;
    QDataStream out(&upBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    //保留总大小信息空间、文件基本信息空间，然后输入文件基本信息
    out << qint64(0) << qint64(0) << upFileInfo.fileSize   << upFileInfo.fileType
        << upFileInfo.fileName    << upFileInfo.targetPath << upFileInfo.uploadTime;
    //这里的总大小是总大小信息、文件名基本信息、和实际文件大小的总和
    upTotalBytes += upBlock.size();
    //要发送的整个数据的大小（文件头结构+实际文件大小）  放在数据流最开始，占用第一个qint(64)的空间
    out.device()->seek(0);
    //返回outBlock的开始，用实际的大小信息代替两个qint(0)空间
    out << upTotalBytes << qint64((upBlock.size() - sizeof(qint64) * 2));

    //已经发送的数据大小和剩余要发送的数据大小
    //upBytesWritten = upBlock.size();
    upBytesToWrite = upTotalBytes - tcpSocket->write(upBlock);
    upBlock.resize(0);  //outBlock是暂存数据的，最后要将其清零
    isUploadingFile = true;     //修改正在上传文件标记
    qDebug() <<"正在上传文件";
}


/*传输文件給服务器*/
void DataTransfer::onSendFileToServer(qint64 numBytes)
{
    if(isUploadingFile){
        upBytesWritten += numBytes;
        qint64 upPerSize = 64 * 1024;
        //如果已经发送了数据
        if(upBytesToWrite > 0){
            //每次发送payloadSize大小的数据，这里设置为64KB，如果剩余的数据不足64KB就发送剩余数据的大小
            upBlock = fileToUpload->read(qMin(upBytesToWrite, upPerSize));
            //发送完一次数据后还剩余数据的大小
            upBytesToWrite -= (qint64)tcpSocket->write(upBlock);
            //清空发送缓冲区
            upBlock.resize(0);
        }
        else{   //如果没有发送任何数据，则关闭文件
            fileToUpload->close();
        }

        //如果发送完毕
        if(upBytesWritten == upTotalBytes){
            //int realNameIndex = upFileInfo.fileName.lastIndexOf("/");
            //QString realName = upFileInfo.fileName.right(upFileInfo.fileName.length ()-realNameIndex-1);  //取真正文件名
            fileToUpload->close();
            isUploadingFile = false;    //退出文件传输状态
            //emit uploadComplete();
            //qDebug() <<"文件传输完成";
        }
    }
}

/*------------------------处理上传文件功能模块结束--------------------------*/


/*------------------------处理下载文件功能模块-----------------------------*/


/*接收MyFile界面发出的下载文件信号，将目标路径发送给服务器*/
void DataTransfer::onDownloadFile(QString targetPath)
{

    downloadPath = targetPath;
    qDebug() <<"download path : " << downloadPath;
    emit sendRequestToServer(DOWNLOAD_REQ, targetPath, "");
}


/*接收服务器传输的文件*/
void DataTransfer::getFileFromServer()
{
    if(!getFileHead()) return;
    getFileContent();
}


/*接收文件头,如果文件头不完整就会返回false,等待下次接收*/
bool DataTransfer::getFileHead()
{
    //tcpSocket->waitForReadyRead(-1);                    //阻塞线程
    //qDebug() << "正在接收文件";
    bool flag = true;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_15);
    if(downBytesReceived <= sizeof(qint64) * 2){        //接收文件头
        if(tcpSocket->bytesAvailable() >= sizeof(qint64) * 2 && downFileInfoSize == 0){
            in >> downTotalBytes >> downFileInfoSize;
            downBytesReceived += sizeof(qint64) * 2;
        }
        if(tcpSocket->bytesAvailable() >= downFileInfoSize && downFileInfoSize != 0){
            //接收文件信息
            in >> downFileInfo.fileSize    >> downFileInfo.fileType >> downFileInfo.fileName
                >> downFileInfo.targetPath >> downFileInfo.uploadTime;

            //输出文件信息
            qDebug() << "文件名："   << downFileInfo.fileName;
            qDebug() << "文件大小：" << downFileInfo.fileSize << "字节";
            qDebug() << "文件类型：" << downFileInfo.fileType;
            qDebug() << "目标路径：" << downFileInfo.targetPath;
            qDebug() << "上传时间：" << downFileInfo.uploadTime;

            if(isFileList){
                //isFileList = false;
                downFileInfo.filePath = fileListPath;
            }else{
                //QString fullPuth = QDir::toNativeSeparators(downloadPath);
                downFileInfo.filePath = QDir(downloadFolder).filePath(downloadPath);
            }

            /*   fileToDownload = new QFile(downFileInfo.filePath);
            if(!fileToDownload->open(QFile::WriteOnly)){   //若文件不存在，会自动创建一个
                qDebug() << "Client: open file error!";
            }*/
            QFile temp(downFileInfo.filePath);
            if(!temp.open(QFile::WriteOnly)){   //若文件不存在会直接创建一个
                qDebug() <<"这个文件应该直接放在download文件夹下";
                downFileInfo.filePath = QDir(downloadFolder).filePath(downFileInfo.fileName);
            }else{
                temp.close();
            }
            fileToDownload = new QFile(downFileInfo.filePath);
            if(!fileToDownload->open(QFile::WriteOnly)){   //若文件不存在，会自动创建一个
                qDebug() << "Client: open file error!";
            }
            qDebug() << "下载文件在客户端中的路径: " << downFileInfo.filePath;

            downBytesReceived += downFileInfoSize;
        }else{                                          //返回，等待接收完整的文件头信息
            flag = false;
        }
    }
    return flag;
}


/*接收文件内容*/
void DataTransfer::getFileContent()
{
    //文件未接收完时继续接收
    if(downBytesReceived < downTotalBytes){
        downBytesReceived += tcpSocket->bytesAvailable();
        downBlock = tcpSocket->readAll();
        fileToDownload->write(downBlock);
        downBlock.resize(0);
    }

    //数据接收完成
    if(downBytesReceived == downTotalBytes){
        fileToDownload->close();
        downTotalBytes = 0;
        downBytesReceived = 0;
        isDownloadingFile = false;                      //设置为不接收文件
        if(isFileList){
            isFileList = false;
            emit fileListDisplayToMyFile();
        }else{
            emit downloadComplete();
        }

        qDebug() << "完成接收";
    }
}
/*------------------------处理下载文件功能模块结束--------------------------*/
