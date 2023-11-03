#include "myfile.h"
#include "ui_myfile.h"


#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QListView>
#include <QInputDialog>


MyFile::MyFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyFile)
{
    ui->setupUi(this);
    dir = new QDir();

    currentPath = "/";                   //记录用户目前所在文件夹，默认为根目录下

    QString appDir = QCoreApplication::applicationDirPath();
    QString nativeAppDir = QDir::toNativeSeparators(appDir);
    fileListPath = QDir(nativeAppDir).filePath(".fileList");
    downFolder = QDir(nativeAppDir).filePath("download");

    //qDebug() <<"fileListPath: " <<fileListPath;

    dataModel = new MyDataModel(this);

    //设置地址栏为不可编辑
    ui->editFilePath->setReadOnly(true);
    updateMyFile();

    connect(this, &MyFile::fileListDisplay, this, &MyFile::onFileListDisplay);

    connect(ui->tableView, &QTableView::doubleClicked, this, &MyFile::onTableRowClicked);


}

MyFile::~MyFile()
{
    delete ui;
    delete dir;
    delete dataModel;

}

/*更新界面*/
void MyFile::updateMyFile()
{
    ui->editFilePath->setText(currentPath);
    emit fileListDisplay();
}


/*返回转换后的标准路径*/
QString MyFile::convertPath(QString filePath)
{
    QDir dir(filePath);

    // 去除最前面的"/"目录
    if (dir.isAbsolute() && dir.path().startsWith("/")) {
        filePath = dir.path().mid(1);
    }
    //QString targetPath = QDir::toNativeSeparators(filePath);
    //return targetPath;
    return filePath;
}


/*显示用户点击的文件夹*/
void MyFile::onTableRowClicked(const QModelIndex &index)
{
    // 检查索引的有效性
    if (index.isValid()) {
        // 获取用户点击的行号
        int clickedRow = index.row();

        // 获取点击行的数据
        QStringList rowData = dataModel->getRowData(clickedRow);

        QString fileName = rowData.at(0);

        currentPath = QDir(currentPath).filePath(fileName);

        updateMyFile();
    }
}



/*显示文件列表*/
void MyFile::onFileListDisplay() {
    // 清空现有数据
    // ui->tableView->clear(); // QTableView 没有 clear 函数
    QFile file(fileListPath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() <<"display fileList";
        QString fileListContent = file.readAll();
        file.close();

        // 将 JSON 字符串转换为 JSON 文档
        QJsonDocument doc = QJsonDocument::fromJson(fileListContent.toUtf8());

        // 获取 JSON 对象中的 files 数组
        QJsonArray filesArray = doc.object()["files"].toArray();

        QVector<QStringList> tableData = convertJsonArrayToVector(filesArray);
        // 创建 MyDataModel 实例并设置数据
        //qDebug() << "Table Data: " << tableData;


        dataModel->setData(tableData);

        // 将 MyDataModel 设置为 QTableView 的数据模型
        ui->tableView->setModel(dataModel);
        //设置自适应列宽度
        ui->tableView->resizeColumnsToContents();

    } else {
        qDebug() << "Failed to open file: " << fileListPath;
    }
}


/*遍历 JSON 数组，获取文件信息并添加到数据模型中*/
QVector<QStringList> MyFile::convertJsonArrayToVector(const QJsonArray &filesArray)
{

    QVector<QStringList> data;
    foreach (const QJsonValue &value, filesArray) {
        //qDebug() << "convert";
        QJsonObject fileObject = value.toObject();
        QStringList rowData;
        QString fileType;
        QString path = fileObject["path"].toString();
        QString fileName = fileObject["name"].toString();

        QString directory = path.left(path.lastIndexOf(fileName));
        // 判断directory是否以分隔符结尾，如果是，去除结尾的分隔符
        if ( directory.size() > 1 && directory.endsWith('/'))
        {
            directory.chop(1);
        }

        //qDebug() << "directory: " << directory <<" path: " << path;
        if(directory != currentPath) continue;

        if(fileObject["fileType"].toInt() == 1){        //文件夹
            fileType = "folder";
        }else{                                          //其他文件
            fileType = "file";
        }
        rowData << fileObject["name"].toString()
                << fileObject["uploadTime"].toString()
                << fileType
                << fileObject["size"].toString()
                << path;

        /*qDebug() << "Name: " << fileObject["name"].toString();
        qDebug() <<"File Path" << fileObject["path"].toString();

        qDebug() << "Directory: " << directory;

        qDebug() << "Upload Time: " << fileObject["uploadTime"].toString();
        qDebug() << "Type: " << fileType;
        qDebug() << "Size: " << fileObject["size"].toString();*/

        data.append(rowData);
    }
    return data;
}


void MyFile::on_btnUpload_clicked()
{
    // 弹框询问用户选择文件还是文件夹
    QMessageBox msgBox;
    msgBox.setText("请选择文件或文件夹");
    msgBox.addButton("文件", QMessageBox::AcceptRole);
    msgBox.addButton("文件夹", QMessageBox::AcceptRole);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int choice = msgBox.exec();
    QString filePath;

    if (choice == 0) { // 用户选择了"文件"
        // 打开文件选择对话框，允许选择文件
        filePath = QFileDialog::getOpenFileName(this, "选择文件", "/", "All Files (*);;Directories (*)");
    } else if (choice == 1) { // 用户选择了"文件夹"
        // 打开文件夹选择对话框，允许选择文件夹
        filePath = QFileDialog::getExistingDirectory(this, "选择文件夹", "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    } else { // 用户取消选择
        qDebug() << "用户取消选择文件或文件夹";
        return;
    }

    qDebug() << "选择的文件或文件夹路径：" << filePath;

    // 在这里处理所选文件或文件夹的路径（filePath）
    if(!filePath.isEmpty()){
        // 检查文件是否存在
        if (!QFile::exists(filePath)) {
            QMessageBox::warning(this, tr("错误"), tr("文件不存在"));
            return;
        }
        FileInfo fileInfo;
        //获取文件信息
        QFile file(filePath);

        // 获取文件信息
        QFileInfo info(file);

        fileInfo.fileName = info.fileName();
        fileInfo.fileSize = info.size();
        if(info.isDir()){           //1表示为文件夹，其他根据类型进行判断
            fileInfo.fileType = 1;
        }else{
            fileInfo.fileType = 2; //普通文件
        }
        fileInfo.filePath = info.filePath();
        //根据当前操作系统的规则合并两个路径
        if(currentPath.isEmpty()) fileInfo.targetPath = fileInfo.fileName;
        else fileInfo.targetPath = QDir(currentPath).filePath(info.fileName());
        fileInfo.uploadTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        file.close(); // 关闭文件

        /*qDebug() <<"file name" << fileInfo.fileName;
        qDebug() <<"file tyep" << fileInfo.fileType;
        qDebug() <<"targetPath" << fileInfo.targetPath;*/

        if(choice == 0){        //文件
            emit uploadFileInfo(fileInfo);
        }else{                  //文件夹
            upFolderQueue.push_back(fileInfo);
            uploadFolder();
        }
    }

}


/*上传文件夹:
1、文件夹队列的队首出列
2、发送创建文件夹信号，在服务器目标位置创建文件夹
3、遍历当前文件夹，将当前文件夹下的文件夹放入文件夹队列，文件放入文件队列
4、然后依次发送文件队列中的文件
5、如果文件夹队列非空，就调用uploadFile函数知道文件夹队列为空*/
void MyFile::uploadFolder()
{
    qDebug() <<"还剩" << upFolderQueue.size() <<"个文件夹未上传";
    if(!upFolderQueue.empty())
    {
        FileInfo fileInfo = upFolderQueue.front();
        upFolderQueue.pop_front();

        emit createFolderReq(fileInfo.targetPath, fileInfo.fileName);

        // 遍历当前文件夹，将文件夹和文件加入队列
        QDir currentDir(fileInfo.filePath);
        QFileInfoList entries = currentDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        qDebug() <<"entries.size = " << entries.size();
        foreach (const QFileInfo& entry, entries)
        {
            FileInfo entryInfo;
            entryInfo.fileName = entry.fileName();
            entryInfo.filePath = entry.filePath();
            entryInfo.fileSize = entry.size();
            entryInfo.targetPath = QDir(fileInfo.targetPath).filePath(entryInfo.fileName);

            if (entry.isDir()) {
                // 如果是文件夹，加入文件夹队列
                entryInfo.fileType = 1;
                upFolderQueue.push_back(entryInfo);
            } else {
                // 如果是文件，加入文件队列
                entryInfo.fileType = 2;
                upFileQueue.push_back(entryInfo);
            }
            /*qDebug() <<"entryInfo.fileName" << entryInfo.fileName;
            qDebug() <<"entryInfo.filePath" << entryInfo.filePath;
            qDebug() <<"entryInfo.fileType" << entryInfo.fileType;
            qDebug() <<"entryInfo.fileSize" << entryInfo.fileSize;*/
        }

        onUploadFileQueue();
    }else{
        // 文件夹队列为空，上传完成，显示提示框
        QMessageBox::information(this, "上传完成", "上传完成！");
    }

}


/*上传文件队列中的文件*/
void MyFile::onUploadFileQueue()
{
    if(!upFileQueue.empty())              //继续发送文件队列中的文件
    {
        FileInfo fileInfo = upFileQueue.front();
        upFileQueue.pop_front();
        emit uploadFileInfo(fileInfo);
    }else{                              //上传文件夹队列中的文件夹
        uploadFolder();
    }

}


/*接收数据传输组件返回的服务器处理结果,并根据结果进行对应的响应*/
void MyFile::onResultFromDataTransfer(qint8 resultType)
{
    switch (resultType) {
    case FILE_EXISTS:
        // 文件存在，询问用户是否继续上传，覆盖原来的文件
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("File Exists"), tr("The file already exists. Do you want to overwrite it?"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                // 用户选择覆盖，执行上传操作
                // uploadFile();
            } else {
                // 用户选择取消上传
                // cancelUpload();
            }
            break;
        }
    case OVER_LIMIT:
        //提醒用户文件大小超出限制
        {
            break;
        }
    case PATH_INCOMPLETE:
        //提醒用户上传的路径残缺，刷新文件目录
        {
            break;
        }
    case CAN_UPLOAD:
        //服务器中父文件夹存在，且待上传的文件大小没有超过限制，可以上传
        {
            //将确认选择上传文件结果发送给数据传输组件
            emit sendChoiceToDataTransfer(SURE_UPLOAD);
            break;
        }
    case CAN_DOWNLOAD:
        //可以下载文件
        {
            emit sendChoiceToDataTransfer(SURE_DOWNLOAD);
            break;
        }
    case FILE_NOT_EXISTS:
        //文件不存在
        {
            break;
        }
    default:
        // 未知的处理结果
        qDebug() << "Unknown result type received from data transfer component: " << resultType;
        break;
    }
}


/*下载选中的文件或者文件夹
 * 1、遍历用户在界面中选中的文件
 * 2、将选中的文件夹压入文件夹下载队列中，将选中的文件压入文件下载队列中
 * 3、文件直接进行下载，下载保存的地址为默认地址
 * 4、如果有文件夹就执行downFolder函数
*/
void MyFile::on_btnDownload_clicked()
{

    QVector<int> selectedRows = dataModel->getSelectedRows();
    foreach (int row, selectedRows) {
        qDebug() << "Selected row: " << row;
        // 这里可以处理选中行的逻辑，例如下载操作
        QStringList rowData = dataModel->getRowData(row);   // 使用数据模型的函数获取选中行的数据

        QString fileType = rowData.at(2);                   // 获取第三列数据（文件类型）
        QString filePath = rowData.at(4);

        qDebug() <<"MyFile filePath:" << filePath;
        if(fileType == "folder")                            //文件夹
        {
            downFolderQueue.push_back(filePath);
        }else{                                              //普通文件
            downFileQueue.push_back(filePath);
        }

    }

    //先处理选中的文件
    onDownloadFileQueue();
}


/*下载downFolderQueue中存储的文件夹*/
void MyFile::downloadFolder()
{
    if(!downFolderQueue.empty())
    {
        QString filePath = downFolderQueue.front();
        downFolderQueue.pop_front();
        /*QString temp = filePath;
        // 将路径标准化
        QDir dir(temp);

        // 去除最前面的"/"目录
        if (dir.isAbsolute() && dir.path().startsWith("/")) {
            temp = dir.path().mid(1);
        }
        QString targetPath = QDir::toNativeSeparators(temp);*/
        QString targetPath = convertPath(filePath);
        createDir(targetPath);

        //遍历当前文件夹，将文件和文件夹分别加入队列中
        QFile file(fileListPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            //qDebug() <<"display fileList";
            QString fileListContent = file.readAll();
            file.close();

            // 将 JSON 字符串转换为 JSON 文档
            QJsonDocument doc = QJsonDocument::fromJson(fileListContent.toUtf8());

            // 获取 JSON 对象中的 files 数组
            QJsonArray filesArray = doc.object()["files"].toArray();

            foreach (const QJsonValue &value, filesArray) {
                //qDebug() << "convert";
                QJsonObject fileObject = value.toObject();
                QString path = fileObject["path"].toString();
                QString fileName = fileObject["name"].toString();

                QString directory = path.left(path.lastIndexOf(fileName));
                qDebug() << "Dir:" << directory;
                // 判断directory是否以分隔符结尾，如果是，去除结尾的分隔符
                if ( directory.size() > 1 && directory.endsWith('/'))
                {
                    directory.chop(1);
                }

                //qDebug() << "directory: " << directory <<" path: " << path;
                if(directory != filePath) continue;

                if(fileObject["fileType"].toInt() == 1){        //文件夹
                    downFolderQueue.push_back(path);
                }else{                                          //其他文件
                    downFileQueue.push_back(path);
                }

            }

        } else {
            qDebug() << "Failed to open file: " << fileListPath;
        }

        onDownloadFileQueue();
    }else{
        // 文件夹队列为空，下载完成，显示提示框
        QMessageBox::information(this, "下载完成", "文件下载完成！");
    }
}


/*下载downFileQueue中的文件*/
void MyFile::onDownloadFileQueue()
{
    if(!downFileQueue.empty()) {
        QString  filePath = downFileQueue.front();
        downFileQueue.pop_front();
        qDebug() <<"front path:" << filePath;
        QString targetPath = convertPath(filePath);
        qDebug() <<"targetPath:" <<targetPath;
        emit downloadFileReq(targetPath);
    }else{
        downloadFolder();
    }
}

/*在指定位置创建文件夹*/
void MyFile::createDir(QString folderPath)
{
    // 创建文件夹
    QString fullPath = QDir(downFolder).filePath(folderPath);
    qDebug() <<"full path:" << fullPath;
    QDir dir(fullPath);

    if (!dir.exists()) {
        if (dir.mkpath(".")) {
            qDebug() << "文件夹创建成功：" << folderPath;
        } else {
            qDebug() << "文件夹创建失败：" << folderPath;
        }
    } else {
        qDebug() << "文件夹已经存在：" << folderPath;
    }
}


/*在当前目录下新建文件夹*/
void MyFile::on_btnNewDir_clicked()
{
    bool ok;
    QString folderName = QInputDialog::getText(this, tr("输入文件夹名"), tr("文件夹名:"), QLineEdit::Normal, "", &ok);

    if (ok && !folderName.isEmpty()) {
    // 用户点击了OK按钮，并且输入不为空
        qDebug() << "用户输入的文件夹名：" << folderName;
        QString targetPath = QDir(currentPath).filePath(folderName);

        emit createFolderReq(targetPath, folderName);
    } else {
    // 用户点击了取消按钮或者输入为空
        qDebug() << "用户取消输入或输入为空";
    }
}


void MyFile::on_btnShare_clicked()
{

}


/*删除按钮*/
void MyFile::on_btnDelete_clicked()
{
    QVector<int> selectedRows = dataModel->getSelectedRows();
    foreach (int row, selectedRows) {
        qDebug() << "Selected row: " << row;
        // 这里可以处理选中行的逻辑，例如下载操作
        QStringList rowData = dataModel->getRowData(row);   // 使用数据模型的函数获取选中行的数据
        QString filePath = rowData.at(4);
        deleteQueue.push_back(filePath);
    }
    onDeleteFile();
}


/*删除文件*/
void MyFile::onDeleteFile()
{
    if(!deleteQueue.empty())
    {
        QString filePath = deleteQueue.front();
        deleteQueue.pop_front();
        QString targetPath = convertPath(filePath);
        emit deleteFileReq(targetPath);
    }else{
        QMessageBox::information(this, "删除完成", "文件删除完成！");
    }
}


/*返回上一级*/
void MyFile::on_btnBack_clicked()
{
    if(currentPath.size() == 1) return;
    QFileInfo fileInfo(currentPath);
    QString fileName = fileInfo.baseName(); // 获取逻辑文件名
    QString directory = currentPath.left(currentPath.lastIndexOf(fileName));
    if(directory.size() > 1){               //去除文件分隔符
        directory = directory.chopped(1);
    }
    currentPath = directory;
    //qDebug() << currentPath;
    updateMyFile();

}


/*刷新文件列表*/
void MyFile::on_btnRefresh_clicked()
{
    emit fileListReq();
}


void MyFile::on_btnSort_clicked()
{

}


void MyFile::on_btnDisplay_clicked()
{

}





