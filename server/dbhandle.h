/*
 * @description: 主要实现与数据库相关的处理。这里没有对数据的合法性进行检测，因为在数据输入端已经进行了数据合法性检查，这里默认数据合法。
 *
 * @todo: 1. 数据库初始化：
 *          init()： 初始化数据库连接，创建用户表(主键为自增长int类型)，检查默认用户是否已存在，如果不存在则插入默认用户（用于测试目的）。
 *          createDirById(qint32 id)： 根据用户ID，在服务器端创建相应的用户文件夹，以及在FileList文件夹内为每个用户创建一个.filelist文件，用于存储用户的文件列表信息。
 *
 *        2. 处理用户注册请求：
 *          handleRegister(const QString username, const QString password)： 处理用户注册请求，检查用户名是否已存在，如果不存在则向数据库中插入新用户的信息，
 *          同时调用createDirById()函数为该用户创建文件夹。
 *
 *        3. 处理用户登录请求：
 *          handleLogin(const QString username, const QString password)： 处理用户登录请求，通过输入的用户名从数据库中获取密码，
 *          然后与用户输入的密码进行比较，如果一致则登录成功，否则返回密码错误信息。
 *
 *        4. 查询功能：
 *          getIdByUsername(const QString username)： 根据用户名查询数据库，获取该用户名对应的用户ID，如果用户名不存在则返回0。
 *
 * @date: 2023/11/02
*/
#ifndef DBHANDLE_H
#define DBHANDLE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
class DBHandle : public QObject
{
    Q_OBJECT
public:
    explicit DBHandle();
    ~DBHandle();
    void init();                                                //数据库初始化
    qint8 handleRegister(const QString, const QString);         //注册处理
    qint8 handleLogin(const QString, const QString);            //登录处理
    qint32 getIdByUsername(const QString);                      //根据用户名获取id
private:    //私有函数
    void createDirById(qint32);                                 //用户注册成功时根据用户id创建一个用户文件夹
signals:

};

#endif // DBHANDLE_H
