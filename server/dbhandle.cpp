#include "dbhandle.h"
#include "constant.h"

#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QDir>
#include <QString>

DBHandle::DBHandle()
{
}


/*关闭数据库*/
DBHandle::~DBHandle()
{
    //m_db.close();
}


/*初始化用户数据库*/
void DBHandle::init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user.db");

    if (!db.open()) {
        QMessageBox::critical(0, "Cannot open database",
                              "Unable to establish a database connection: " + db.lastError().text(), QMessageBox::Cancel);
        return;
    }

    // 创建用户表，使用预处理语句
    QSqlQuery query(db);
    QString createTableSQL = "CREATE TABLE IF NOT EXISTS user (id INTEGER PRIMARY KEY AUTOINCREMENT, username VARCHAR(32), password VARCHAR(32))";
    if (!query.exec(createTableSQL)) {
        qDebug() << "Can't create table user: " << query.lastError().text();
        db.close();
        return;
    }

    // 检查默认用户是否已存在,默认用户是用的我室友的名字🤣
    QSqlQuery checkUserQuery(db);
    checkUserQuery.prepare("SELECT COUNT(*) FROM user WHERE username = :username");
    checkUserQuery.bindValue(":username", "xtc");
    if (!checkUserQuery.exec() || !checkUserQuery.next()) {
        qDebug() << "Error checking if default user exists: " << checkUserQuery.lastError().text();
        db.close();
        return;
    }
    int userCount = checkUserQuery.value(0).toInt();
    if (userCount == 0) {
        // 默认用户不存在，插入
        QSqlQuery insertQuery(db);
        insertQuery.prepare("INSERT INTO user (username, password) VALUES (:username, :password)");
        insertQuery.bindValue(":username", "xtc");
        insertQuery.bindValue(":password", "xtcnb666");

        if (!insertQuery.exec()) {
            qDebug() << "Error inserting default user: " << insertQuery.lastError().text();
        }else{
            createDirById(1);
        }
    }
    db.close();
}


/*根据查询的用户名返回用户id,不存在返回0表示没有这个用户名对应的id，存在则返回id值*/
qint32 DBHandle::getIdByUsername(const QString username)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen())
    {
        qDebug() << "Error: Failed to connect database." << db.lastError();
        return UNKNOWN_ERROR;
    }

    QSqlQuery query(db);
    QString sql = "SELECT id FROM user WHERE username = :username";
    query.prepare(sql);
    query.bindValue(":username", username);

    if (!query.exec())
    {
        qDebug() << "Error executing query:" << query.lastError();
        db.close();
        return UNKNOWN_ERROR;
    }

    qint32 id = 0;
    if (query.first())
    {
        id = query.value(0).toInt();
        qDebug() << username << " id = " << id;
    }

    db.close(); // 关闭数据库连接

    return id;
}


/*处理注册请求,注册用户名如果已经存在则不能注册。返回注册结果。*/
qint8 DBHandle::handleRegister(const QString username, const QString password)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()){
        qDebug() << "Error: Failed to connect to the database." << db.lastError();
        return UNKNOWN_ERROR;
    }
    // 检查用户名是否已存在
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM user WHERE username = :username");
    checkQuery.bindValue(":username", username);
    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Error checking username existence: " << checkQuery.lastError().text();
        db.close();
        return UNKNOWN_ERROR;
    }
    int userCount = checkQuery.value(0).toInt();
    if (userCount > 0) {
        qDebug() << "用户已存在，不能注册";
        return USER_EXISTS;
    }
    // 查询最大ID
    QSqlQuery maxIdQuery(db);
    if (maxIdQuery.exec("SELECT MAX(id) FROM user")) {
        if (maxIdQuery.next()) {
            int maxId = maxIdQuery.value(0).toInt();
            qDebug() << "当前最大的ID为" << maxId;
            // 插入新用户
            QSqlQuery insertQuery(db);
            insertQuery.prepare("INSERT INTO user (id, username, password) VALUES (?, ?, ?)");
            insertQuery.bindValue(0, maxId + 1);
            insertQuery.bindValue(1, username);
            insertQuery.bindValue(2, password);

            if (!insertQuery.exec()) {
                qDebug() << "Error inserting user: " << insertQuery.lastError().text();
                db.close();
                return UNKNOWN_ERROR;
            } else {
                createDirById(maxId + 1);
                qDebug() << "成功插入注册信息";
            }
        }
    } else {
        qDebug() << "查询失败:" << maxIdQuery.lastError().text();
    }
    db.close();
    return REGISTER_SUCCESS;
}


/*处理登录请求，查找数据库中该用户名，若存在则比对密码。返回登录结果。*/
qint8 DBHandle::handleLogin(const QString username, const QString password)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()){
        qDebug() << "Error: Failed to connect database." << db.lastError();
        return UNKNOWN_ERROR;
    }

    QSqlQuery query(db);
    QString sql = "SELECT password FROM user WHERE username = :username";
    query.prepare(sql);
    query.bindValue(":username", username);

    if (!query.exec()){
        qDebug() << "Login query execution error:" << query.lastError();
        db.close(); // 关闭数据库连接
        return UNKNOWN_ERROR;
    }

    if (!query.next()){
        qDebug() << "登录失败，用户名错误";
        db.close(); // 关闭数据库连接
        return LOGIN_FAILED;
    }else{
        QString passwordFromDB = query.value(0).toString();

        if (passwordFromDB == password){
            qDebug() << "登录成功";
            db.close(); // 关闭数据库连接
            return LOGIN_SUCCESS;
        }else{
            qDebug() << "登录密码错误";
            db.close(); // 关闭数据库连接
            return PASSWORD_ERROR;
        }
    }
}


/*根据用户id，在User文件夹内为对应用户创建文件夹,在FileList文件夹内创建.filelist文件*/
void DBHandle::createDirById(qint32 id)
{
    QString userId = QString::number(id);
    // 构建用户文件夹路径
    QString appDir = QCoreApplication::applicationDirPath();
    QString nativeAppDir = QDir::toNativeSeparators(appDir);
    QString userDirPath = QDir(nativeAppDir).filePath("User");
    QString fileListDirPath = QDir(nativeAppDir).filePath("FileList");
    QString userFolderPath = QDir(userDirPath).filePath(userId);
    QString fileListFolderPath = QDir(fileListDirPath).filePath(userId);

    // 检查用户文件夹是否存在，如果不存在就创建它
    QDir userDir(userFolderPath);
    if (!userDir.exists()) {
        if (userDir.mkpath(".")) {
            qDebug() << "User folder created for user ID:" << userId;
        } else {
            qDebug() << "Failed to create user folder for user ID:" << userId;
        }
    } else {
        //qDebug() << "User folder already exists for user ID:" << userId;
    }

    //检查FileList文件夹下的用户文件夹是否存在，如果不存在就创建它
    QDir fileListDir(fileListFolderPath);
    if(!fileListDir.exists()){
        if (fileListDir.mkpath(".")) {
            qDebug() << "FileList folder created for user ID:" << userId;
        } else {
            qDebug() << "Failed to create FileList folder for user ID:" << userId;
        }
    }else{
        //qDebug() << "FIleList folder already exists for user ID:" << userId;
    }

    //在每个用户对应的filelist文件夹下创建.filelist文件
    QString fileListPath = QDir(fileListFolderPath).filePath(".fileList");
    QFile fileList(fileListPath);
    if(!fileList.exists()){
        if (fileList.open(QIODevice::WriteOnly)) {
            // 关闭文件
            fileList.close();
            qDebug() << "fileList created successfully.";
        } else {
            qDebug() << "Failed to create fileList.";
        }
    }else{
        //qDebug() <<"fileList exiests";
    }

}
