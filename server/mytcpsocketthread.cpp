#include "mytcpsocketthread.h"
#include "constant.h"
#include "dbhandle.h"
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>

MyTcpSocketThread::MyTcpSocketThread()
{  
}


MyTcpSocketThread::~MyTcpSocketThread()
{
    delete socket;
    delete database;
}


/*初始化*/
void MyTcpSocketThread::init()
{
    database = new DBHandle();  // 初始化数据库操作对象
    database->init();
    threadRunning = true;
    isLogin = false;
    isGettingFile = false;                        //初始化为不接收文件
    isDownloadingFile = false;                    //初始化为不下载文件

    userName.clear();
    userDir.clear();
    QString appDir = QCoreApplication::applicationDirPath();
    QString nativeAppDir = QDir::toNativeSeparators(appDir);
    userDir = QDir(nativeAppDir).filePath("User");

    // 与客户端上传文件相关的初始化
    upTotalBytes = 0;
    upBytesReceived = 0;
    upFileInfoSize = 0;
    upFile = nullptr;
    upFileInfo = FileInfo();
    upBlock.clear();

    // 与客户端下载文件相关的初始化
    downTotalBytes = 0;
    downBytesWritten = 0;
    downBytesToWrite = 0;
    downFileInfoSize = 0;
    downFile = nullptr;
    downFileInfo = FileInfo();
    downBlock.clear();

    qDebug()<<"start";
    //建立socket
    socket = new QTcpSocket();
    socket->setSocketDescriptor(ptr);   //将这个连接的标识符赋值给这个socket类
    qDebug() <<socket->state();

    //如果客户端处于传输文件中，每当缓冲区可以容纳新数据，就接收这个信号将文件的下一部分写入缓冲区
    connect(socket, &QIODevice::bytesWritten, this, &MyTcpSocketThread::onSendFileToClient);
    //每当有可读信息就用onReadMessage()函数进行读取
    //connect(socket, &QAbstractSocket::bytesWritten, this, &MyTcpSocketThread::onSendFileToClient);
}


/*线程开始运行*/
void MyTcpSocketThread::run()
{
    init();
    QEventLoop eventLoop;               // 定义 eventLoop 对象
    while(threadRunning){
        //if(socket->bytesAvailable() > 0 )qDebug() << "接收到的数据大小: " << socket->bytesAvailable();
        if(!isGettingFile){             //接收请求
            getRequestFromClient();
        }else{                          //传输文件
            //getFileFromClient();
            // 如果需要等待数据到达，进入事件循环
            if (socket->bytesAvailable() == 0) {
                connect(socket, &QIODevice::readyRead, &eventLoop, &QEventLoop::quit);
                eventLoop.exec();
            }
            getFileFromClient(); // 处理文件传输
        }//end if
    }//end while
}


/*-------------------------客户端通信模块--------------------------*/

/*发送信息给客户端*/
void MyTcpSocketThread::sendResultToClient(qint8 messageType)
{
    QByteArray messageBlock;
    QDataStream out(&messageBlock, QIODevice::WriteOnly);       //用于将请求信息序列化(就是将请求信息转换成二进制信息)
    out.setVersion(QDataStream::Qt_5_15);                       //设置数据流版本
    //预先填入一个8字节的0，因为设计的协议前8个字节表示数据包大小
    out <<(qint64)0;
    out << messageType;                                         //输入实际数据
    out.device()->seek(0);                                      //跳转到数据头部
    out << (qint64)(messageBlock.size() - sizeof(qint64));      //实际数据的大小
    socket->write(messageBlock);                                //发送数据
    qDebug() << "待发送的数据大小：" <<socket->bytesToWrite();
    socket->waitForBytesWritten();                              //等待数据传输完成
    if(socket->bytesToWrite() == 0) qDebug() <<"server send result over. "<< "Result is " << messageType ;
    messageBlock.resize(0);                                     //清空
}


/*从客户端接收信息*/
void MyTcpSocketThread::getRequestFromClient()
{
    qDebug() <<"\n";
    qint64 dataSize = 0;
    qint8 dataType = 0;
    /*调用waitForReadyRead()函数后，程序会进入阻塞状态，直到有可读取的数据到达或者超时发生。该函数返回true表示已经有可读取的数据到达，
     * 可以使用read()函数进行读取操作；返回false表示等待超时，或者在等待期间发生错误。要注意应用程序在等待期间可能变得不响应，所以后续
     * 完善程序时会设置合理的超时时间来保证程序的响应性能。*/
    socket->waitForReadyRead(-1);
    if(socket->bytesAvailable() > 0 )qDebug() <<"接收的数据大小：" <<socket->bytesAvailable();
    QString str1;               //请求中的第一个字符串
    QString str2;               //请求中的第二个字符串
    QDataStream in(socket);     //用来读取服务器从这个连接接收到的二进制数据流
    in.setVersion(QDataStream::Qt_5_15);    //设置数据流版本
    if(dataSize == 0){          //如果是刚开始接收数据
        /*判断接收的数据是否大于四个字节，也就是文件的大小信息所占的空间
          如果是则保存到requestSize变量中，否则直接返回，继续接收数据*/
        if(socket->bytesAvailable() < (qint64)sizeof(qint64)) return;
        in >> dataSize;
    }

    if(socket->bytesAvailable() < dataSize) return; //如果没有得到全部的数据，则返回，继续接收数据

    in >> dataType >> str1 >> str2;     //将接收到的数据放在变量中
    qDebug() <<"get message from client:" <<dataType <<str1 <<str2;
    //根据接收到的请求类型进行处理
    handleRequest(dataType, str1, str2);
}


/*处理来自客户端的请求，第一个参数为请求类型，后面的为请求信息*/
void MyTcpSocketThread::handleRequest(qint8 requestType, QString str1, QString str2)
{
    switch (requestType) {
    case EXIT_APPLICATION:
        //退出登录
        logoutExecution();
        break;
    case REGISTER_REQ:
        //注册请求
        registerExecution(str1, str2);
        break;
    case LOGIN_REQ:
        //登录请求
        loginExecution(str1, str2);
        break;
    case UPLOAD_REQ:
        //请求上传文件
        getUploadFileRequest(str1, str2);
        break;
    case SURE_UPLOAD:
        //确认上传文件
        isGettingFile = true;
        break;
    case CANCEL_UPLOAD:
        //取消上传文件
        break;
    case DOWNLOAD_REQ:
        //请求下载文件
        getDownloadFileRequest(str1);
        break;
    case SURE_DOWNLOAD:
        //确认下载
        sendFileInfoToClient();
        break;
    case CANCEL_DOWNLOAD:
        //取消下载
        break;
    case FILE_LIST_REQ:
        //文件列表传输请求
        handleFileListReq();
        break;
    case CREATE_FOLDER_REQ:
        //创建文件夹请求
        createFolder(str1, str2);
        break;
    case DELETE_FILE_REQ:
        //删除文件请求
        deleteFile(str1);
        break;
    default:
        break;
    }

}
/*-------------------------客户端通信模块结束--------------------------*/


/*-------------------------客户端请求处理模块--------------------------*/

/*用户退出登录*/
void MyTcpSocketThread::logoutExecution()
{
    socket->close();                    //关闭socket
    threadRunning = false;              //关闭线程
}


/*注册新用户*/
void MyTcpSocketThread::registerExecution(QString username, QString password)
{
    qint8 result = database->handleRegister(username, password);
    sendResultToClient(result);
}


/*用户登录*/
void MyTcpSocketThread::loginExecution(QString username, QString password)
{
    qint8 result = database->handleLogin(username, password);
    if(result == LOGIN_SUCCESS){
        isLogin = true;
        userName = username;
        QString userId = QString::number(database->getIdByUsername(username));
        QString native = QDir::toNativeSeparators(userDir);
        userDir = QDir(native).filePath(userId);

        QString appDir = QCoreApplication::applicationDirPath();
        QString nativeAppDir = QDir::toNativeSeparators(appDir);
        QString fileListFolder = QDir(nativeAppDir).filePath("FileList");
        QString fileListDir = QDir(fileListFolder).filePath(userId);
        fileListPath = QDir(fileListDir).filePath(".fileList");
        qDebug() << "Login success. FileListPath: " << fileListPath;
    }
    sendResultToClient(result);
}


/*以json文件形式保存用户文件列表到fileListPath地址的文件中*/
void MyTcpSocketThread::saveFileList()
{
    // 创建一个 JSON 对象，用于保存文件夹目录结构, 创建一个 JSON 数组，用于保存文件夹和文件的信息
    QJsonObject fileListObject;
    QJsonArray filesArray;

    // 使用 QDirIterator 遍历用户文件夹中的所有文件和文件夹,QDirIterator::Subdirectories：表示递归遍历子目录
    QDirIterator it(userDir, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();

        // 创建一个 JSON 对象，保存当前文件或文件夹的信息
        QJsonObject fileObject;
        fileObject["name"] = fileInfo.fileName();

        // 确保文件路径以用户目录开始
        if (fileInfo.filePath().startsWith(userDir)) {
            // 获取相对路径，将相对路径存储到fileObject的"path"键中
            QString relativePath = fileInfo.filePath();
            relativePath.remove(0, userDir.length());
            qDebug() << "Relative Path:" << relativePath;
            fileObject["path"] = relativePath;
        } else {
            qDebug() << "Error: filePath does not start with userDir";
        }

        fileObject["fileType"] = fileInfo.isDir() ? 1 : 2;                                  // 1 表示文件夹，2 表示文件

        fileObject["size"] = fileInfo.isDir() ? "-": QString::number(fileInfo.size());      // 如果是文件夹则为默认大小-1，如果是文件，保存文件大小到 JSON 对象中的 "size" 键

        fileObject["uploadTime"] = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"); // 将文件或文件夹的创建时间保存到 JSON 对象中的 "uploadTime" 键

        // 将当前文件或文件夹的 JSON 对象添加到 JSON 数组中
        filesArray.append(fileObject);
    }

    /* 将 JSON 数组保存到 JSON 对象中的 "files" 键;创建一个 JSON 文档，用于保存 JSON 对象中的数据;
     * 将 JSON 文档转换为 JSON 字符串，并保存在 fileList 变量中*/
    fileListObject["files"] = filesArray;
    QJsonDocument doc(fileListObject);
    QString fileList = doc.toJson(QJsonDocument::Compact);

    // 测试代码：输出 fileList 的内容
//    qDebug() << "File List JSON:";
//    qDebug().noquote() << fileList;
    //将json写入.fileList文件中，编码格式为utf-8
    QFile file(fileListPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << fileList;
        file.close();
        qDebug() << "File List saved to file: " << fileListPath;
    } else {
        qDebug() << "Failed to open file for writing: " << fileListPath;
    }

}


/*处理客户端文件列表请求*/
void MyTcpSocketThread::handleFileListReq()
{
    //第一步：发送文件列表标识符，告诉客户端将发送文件列表
    sendResultToClient(FILE_LIST_INFO);

    //第二步：获取用户文件列表信息，并保存到指定路径文件中
    saveFileList();

    //第三步：获取文件信息
    QFile fileList(fileListPath);
    if(!fileList.exists()){
        qDebug() <<"文件不存在";
    }else{
        //待下载的文件存在，记录文件信息
        if (fileList.open(QIODevice::ReadOnly)) {
            // 获取文件信息
            QFileInfo info(fileList);

            downFileInfo.fileName = info.fileName();
            downFileInfo.fileSize = info.size();
            if(info.isDir()){           //1表示为文件夹，其他根据类型进行判断
                downFileInfo.fileType = 1;
            }else{
                downFileInfo.fileType = 2; //普通文件
            }
            downFileInfo.filePath = info.filePath();
            downFileInfo.targetPath = downFileInfo.fileName;
            downFileInfo.uploadTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

            // 输出变量的值
            qDebug() << "File Name: " << downFileInfo.fileName;
            qDebug() << "File Size: " << downFileInfo.fileSize;
            qDebug() << "File Type: " << downFileInfo.fileType;
            qDebug() << "File Path: " << downFileInfo.filePath;
            qDebug() << "Target Path: " << downFileInfo.targetPath;
            qDebug() << "Upload Time: " << downFileInfo.uploadTime;

            fileList.close(); // 关闭文件
        } else {
            // 文件打开失败，处理错误逻辑
            qDebug() << "无法打开文件：" << fileList.errorString();
        }
    }

    //第四步：将文件信息发送到客户端，修改服务器状态为正在传输文件
    sendFileInfoToClient();
}


/*返回转换后的标准路径*/
QString MyTcpSocketThread::convertPath(QString filePath)
{
    // 将路径标准化
    QDir dir(filePath);

    // 去除最前面的"/"目录
    if (dir.isAbsolute() && dir.path().startsWith("/")) {
        filePath = dir.path().mid(1);
    }
    QString nativePath = QDir::toNativeSeparators(filePath);
    QString fullPath = QDir(userDir).filePath(nativePath);
    return fullPath;

}


/*创建文件夹*/
void MyTcpSocketThread::createFolder(QString targetPath,QString folderName)
{
    QString fullPath = convertPath(targetPath);
    qDebug() << "待创建的文件夹路径 " << fullPath;

    // 创建文件夹
    QDir folder(fullPath);
    if (!folder.exists()) {
        if (folder.mkpath(".")) {
            qDebug() << "文件夹创建成功：" << fullPath;
        } else {
            qDebug() << "文件夹创建失败：" << fullPath;
        }
    } else {
        qDebug() << "文件夹已存在：" << fullPath;
    }
}


/*删除目标路径下文件或者文件夹*/
void MyTcpSocketThread::deleteFile(QString targetPath)
{
    QString fullPath = convertPath(targetPath);
    QFileInfo fileInfo(fullPath);

    if (fileInfo.isDir()) {
        // 如果是文件夹，使用 QDir 删除
        QDir dir(fullPath);
        if (dir.exists()) {
            if (dir.removeRecursively()) {
                qDebug() << "Folder deleted successfully:" << fullPath;
            } else {
                qDebug() << "Error deleting folder:" << fullPath;
            }
        } else {
            qDebug() << "Folder not found:" << fullPath;
        }
    } else if (fileInfo.isFile()) {
        // 如果是文件，使用 QFile 删除
        QFile file(fullPath);
        if (file.exists()) {
            if (file.remove()) {
                qDebug() << "File deleted successfully:" << fullPath;
            } else {
                qDebug() << "Error deleting file:" << fullPath;
            }
        } else {
            qDebug() << "File not found:" << fullPath;
        }
    } else {
        // 如果既不是文件夹也不是文件
        qDebug() << "Invalid path:" << fullPath;
    }
    sendResultToClient(DELETE_FILE_COMPLETE);
}

/*-------------------------客户端请求处理模块结束--------------------------*/

/*-------------------------接收文件模块----------------------------------*/

/*接收用户上传文件请求*/
void MyTcpSocketThread::getUploadFileRequest(QString fileSize, QString targetPath)
{
    qint8 result = CAN_UPLOAD;
    //int dataSize = fileSize.toLongLong();
    /*
     * 这里进行大小判断，用户待上传的文件大小是否超过限制还是其他约束条件。
     * 目前只是实验阶段，所以没有判断，默认可以上传。
    */
    // 将路径标准化
    /*QDir dir(targetPath);

    // 去除最前面的"/"目录
    if (dir.isAbsolute() && dir.path().startsWith("/")) {
        targetPath = dir.path().mid(1);
    }
    QString nativePath = QDir::toNativeSeparators(targetPath);
    QString fullPath = QDir(userDir).filePath(nativePath);*/
    QString fullPath = convertPath(targetPath);
    qDebug() << "待上传文件的完整路径 " << fullPath;
    upFileInfo.filePath = fullPath;

    // 判断待上传的文件是否存在,不存在就创建一个
    QFile file(fullPath);
    if (!file.exists()) {
        // 文件不存在，创建文件
        if (file.open(QIODevice::WriteOnly)) {
            // 文件创建成功
            qDebug() << "File created: " << fullPath;
            file.close();
        } else {
            // 文件创建失败
            qDebug() << "Failed to create file: " << fullPath;
            result = UNKNOWN_ERROR;
        }
    } else {
        // 文件已存在
        qDebug() << "Target file exists.";
        result = FILE_EXISTS;
    }
    file.close();
    // 如果以上条件都满足，目标文件可以上传
    if(result == CAN_UPLOAD)qDebug() << "Target path is valid.";

    sendResultToClient(result);
}


/*接收用户上传的文件*/
void MyTcpSocketThread::getFileFromClient()
{
    if(!getFileHead()) return;
    getFileContent();
}

/*接收文件头*/
bool MyTcpSocketThread::getFileHead()
{
    bool flag = true;
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    if(upBytesReceived <= sizeof(qint64) * 2){
        if(socket->bytesAvailable() >= sizeof(qint64) * 2){
            in >> upTotalBytes >> upFileInfoSize;
            upBytesReceived += sizeof(qint64) * 2;
        }
        if(socket->bytesAvailable() >= upFileInfoSize && upFileInfoSize != 0){
            //接收文件信息
            in >> upFileInfo.fileSize   >> upFileInfo.fileType   >> upFileInfo.fileName
                >> upFileInfo.targetPath >> upFileInfo.uploadTime;

            // 输出文件信息
            qDebug() << "文件名："   << upFileInfo.fileName;
            qDebug() << "文件大小：" << upFileInfo.fileSize << "字节";
            qDebug() << "文件类型：" << upFileInfo.fileType;
            qDebug() << "目标路径：" << upFileInfo.targetPath;
            qDebug() << "上传时间：" << upFileInfo.uploadTime;
            qDebug() <<"上传文件在服务器中的路径：" <<upFileInfo.filePath;

            upFile = new QFile(upFileInfo.filePath);
            if(!upFile->open(QFile::WriteOnly)){   //若文件不存在，会自动创建一个
                qDebug() << "Server: open file error!";
            }
            upBytesReceived += upFileInfoSize;
        }else{
            flag = false;
        }
    }
    return flag;
}


/*接收文件内容*/
void MyTcpSocketThread::getFileContent()
{
    //文件未接收完时继续接收
    if(upBytesReceived < upTotalBytes){
        upBytesReceived += socket->bytesAvailable();
        upBlock = socket->readAll();
        upFile->write(upBlock);
        upBlock.resize(0);
    }

    //接收数据完成时
    if(upBytesReceived == upTotalBytes){
        upFile->close();
        upTotalBytes = 0;
        upBytesReceived = 0;
        isGettingFile = false;  //状态设置为不接收文件
        qDebug() << "完成接收";
        sendResultToClient(FILE_TRANSFER_COMPLETE);
    }
}

/*-------------------------接收文件模块结束-------------------------------*/


/*-------------------------下载文件模块-------------------------------*/

/*处理文件下载请求，判断目标路径文件是否存在*/
void MyTcpSocketThread::getDownloadFileRequest(QString targetPath)
{

    qint8 result = CAN_DOWNLOAD;
    QString nativePath = QDir::toNativeSeparators(targetPath);
    QString fullPath = QDir(userDir).filePath(nativePath);
    qDebug() << "待下载文件的完整路径 " << fullPath;

    //判断待下载的文件是否存在，不存在就返回错误信息给客户端
    QFile file(fullPath);
    if(!file.exists()){
        qDebug() <<"文件不存在";
        result = FILE_NOT_EXISTS;
    }else{
        //待下载的文件存在，记录文件信息
        if (file.open(QIODevice::ReadOnly)) {
            // 获取文件信息
            QFileInfo info(file);

            downFileInfo.fileName = info.fileName();
            downFileInfo.fileSize = info.size();
            if(info.isDir()){           //1表示为文件夹，其他根据类型进行判断
                downFileInfo.fileType = 1;
            }else{
                downFileInfo.fileType = 2; //普通文件
            }
            downFileInfo.filePath = info.filePath();
            downFileInfo.targetPath = downFileInfo.fileName;
            downFileInfo.uploadTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

            file.close(); // 关闭文件
        } else {
            // 文件打开失败，处理错误逻辑
            qDebug() << "无法打开文件：" << file.errorString();
            result = UNKNOWN_ERROR;
        }
    }

    sendResultToClient(result);
}


/*将请求下载的文件的文件信息发送给客户端*/
void MyTcpSocketThread::sendFileInfoToClient()
{

    downFile = new QFile(downFileInfo.filePath);
    if(!downFile->open(QFile::ReadOnly)){
        qDebug() <<" server: open file error";
        return;
    }else{
        qDebug() << "文件打开成功";
    }
    downTotalBytes = downFileInfo.fileSize;
    QDataStream out(&downBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    //保留总大小信息空间、文件基本信息空间，然后输入文件基本信息
    out << qint64(0) << qint64(0) << downFileInfo.fileSize   << downFileInfo.fileType
        << downFileInfo.fileName  << downFileInfo.targetPath << downFileInfo.uploadTime;

    //这里的总大小是总大小信息、文件名基本信息、和实际文件大小的总和
    downTotalBytes += downBlock.size();
    //要发送的整个数据的大小（文件头结构+实际文件大小）,放在数据流最开始，占用第一个qint(64)的空间
    out.device()->seek(0);
    //返回outBlock的开始，用实际的大小信息代替两个qint(0)空间
    out << downTotalBytes << qint64((downBlock.size() - sizeof(qint64) * 2));

    //已经发送的数据大小和剩余要发送的数据大小
    //downBytesWritten = downBlock.size();
    downBytesToWrite = downTotalBytes - socket->write(downBlock);
//    qDebug() <<"downTotalBytes:" <<downTotalBytes;
//    qDebug() <<"downBytesWritten:" <<downBytesWritten;
//    qDebug() <<"downBytesToWrite:" <<downBytesToWrite;
    downBlock.resize(0);  //outBlock是暂存数据的，最后要将其清零
    isDownloadingFile = true;     //修改正在上传文件标记127.0.0.
    qDebug() <<"正在传输文件";
}


/*将文件发送给客户端*/
void MyTcpSocketThread::onSendFileToClient(qint64 numBytes)
{
    if(isDownloadingFile){
        downBytesWritten += numBytes;
        qint64 upPerSize = 64 * 1024;
        //如果已经发送了数据
        if(downBytesToWrite > 0){
            //每次发送payloadSize大小的数据，这里设置为64KB，如果剩余的数据不足64KB就发送剩余数据的大小
            downBlock = downFile->read(qMin(downBytesToWrite, upPerSize));
            //发送完一次数据后还剩余数据的大小
            downBytesToWrite -= (qint64)socket->write(downBlock);
            //清空发送缓冲区
            downBlock.resize(0);
        }
        else{   //如果没有发送任何数据，则关闭文件
            downFile->close();
        }

        //如果发送完毕
        if(downBytesWritten == downTotalBytes){
            downFile->close();
            isDownloadingFile = false;    //退出文件传输状态
            downTotalBytes = 0;
            downBytesWritten = 0;
            downBytesToWrite = 0;
            qDebug() <<"文件传输完成";
        }
    }
}
/*-------------------------下载文件模块结束-------------------------------*/
