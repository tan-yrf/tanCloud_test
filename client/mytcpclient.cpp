#include "mytcpclient.h"
#include <QCoreApplication>

MyTcpClient::MyTcpClient()
    : QObject{}
{
    createDownloadDir();

    //创建登录注册界面
    login = new Login();
    login->show();
    clientWindow = new ClientWindow();

    //数据传输部分
    dataTransfer = new DataTransfer(this);

    connectLoginWithDataTransfer();
    connectMyFileWithDataTransfer();
    connectSignalWithClient();
    connectClientWindowWithDataTransfer();
}


MyTcpClient::~MyTcpClient()
{
    delete login;
    if(clientWindow != nullptr){
        delete clientWindow;
    }
    delete dataTransfer;
}

/*客户端运行时，如果默认下载文件夹和.fileList文件不存在就创建*/
void MyTcpClient::createDownloadDir()
{
    // 检查User文件夹是否存在，如果不存在就创建它
    QString appDir = QCoreApplication::applicationDirPath();
    QString nativeAppDir = QDir::toNativeSeparators(appDir);
    QString downloadPath = QDir(nativeAppDir).filePath("download");
    //QString fileListPath = QDir(nativeAppDir).filePath(".fileList");

    QDir downloadDir(downloadPath);
    if (!downloadDir.exists()) {
        if (downloadDir.mkpath(".")) {
            qDebug() << "download folder created successfully.";
        } else {
            qDebug() << "Failed to create User folder.";
        }
    } else {
        //qDebug() << "download folder already exists.";
    }

}
/*绑定登录注册页面和数据传输部分的信号与槽*/
void MyTcpClient::connectLoginWithDataTransfer()
{
    //登录注册界面发出信号，数据传输部分接收
    connect(login, &Login::connectServer, dataTransfer, &DataTransfer::onConnectServer);

    connect(login, &Login::closeConnectServer, dataTransfer, &DataTransfer::onCloseConnectServer);

    connect(login, &Login::sendRequestToClient, dataTransfer, &DataTransfer::onSendRequestToServer);

    //数据传输部分发出信号，登录注册界面接收
    connect(dataTransfer, &DataTransfer::sendSocketStateToLogin, login, &Login::onSocketStateFromClient);

    connect(dataTransfer, &DataTransfer::sendResultToLogin, login, &Login::onResultFromClient);

}

/*绑定客户端接收的信号与对应的槽函数*/
void MyTcpClient::connectSignalWithClient()
{
    //客户端接收
    connect(login, &Login::loginSuccess, this, &MyTcpClient::onLoginSuccess);

    //客户端发出
    connect(this, &MyTcpClient::fileListReq, dataTransfer, &DataTransfer::onFileListReq);

}

/*绑定MyFile界面与数据传输部分的信号与槽*/
void MyTcpClient::connectMyFileWithDataTransfer()
{
    //MyFile界面发出信号
    connect(clientWindow->mySpace->myFile, &MyFile::uploadFileInfo,
            dataTransfer, &DataTransfer::onUploadFile);

    connect(clientWindow->mySpace->myFile, &MyFile::sendChoiceToDataTransfer,
            dataTransfer, &DataTransfer::onChoiceFromMyFile);

    connect(clientWindow->mySpace->myFile, &MyFile::downloadFileReq,
            dataTransfer, &DataTransfer::onDownloadFile);

    connect(clientWindow->mySpace->myFile, &MyFile::fileListReq,
            dataTransfer, &DataTransfer::onFileListReq);

    connect(clientWindow->mySpace->myFile, &MyFile::createFolderReq,
            dataTransfer, &DataTransfer::onCreateFolder);

    connect(clientWindow->mySpace->myFile, &MyFile::deleteFileReq,
            dataTransfer, &DataTransfer::onDeleteFileReq);

    //DataTransfer部件发出信号
    connect(dataTransfer, &DataTransfer::sendResultToMyFile,
            clientWindow->mySpace->myFile, &MyFile::onResultFromDataTransfer);

    connect(dataTransfer, &DataTransfer::fileListDisplayToMyFile,
            clientWindow->mySpace->myFile, &MyFile::onFileListDisplay);

    connect(dataTransfer, &DataTransfer::uploadComplete,
            clientWindow->mySpace->myFile, &MyFile::onUploadFileQueue);

    connect(dataTransfer, &DataTransfer::downloadComplete,
            clientWindow->mySpace->myFile, &MyFile::onDownloadFileQueue);

    connect(dataTransfer, &DataTransfer::deleteFileComplete,
            clientWindow->mySpace->myFile, &MyFile::onDeleteFile);
}


/*绑定主界面与数据传输部分的信号与槽*/
void MyTcpClient::connectClientWindowWithDataTransfer()
{
    //主界面发出的信号
    connect(clientWindow, &ClientWindow::closeConnectServer,
            dataTransfer, &DataTransfer::onCloseConnectServer);
}


void MyTcpClient::onLoginSuccess()
{
    login->close();
    //delete login;
    clientWindow->show();
    emit fileListReq();
}
