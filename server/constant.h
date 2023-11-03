#ifndef CONSTANT_H
#define CONSTANT_H

/*存储程序运行中使用的常量*/
#include <QObject>
#include <QFile>

const qint8 EXIT_APPLICATION = -1;              //客户端退出应用
const qint8 UNKNOWN_ERROR = 0;                  //未知错误
const qint8 REGISTER_REQ = 1;                   //注册请求
const qint8 REGISTER_SUCCESS = 2;               //注册成功
const qint8 LOGIN_REQ = 3;                      //登录请求
const qint8 LOGIN_SUCCESS = 4;                  //登录成功
const qint8 LOGIN_FAILED = 5;                   //登录失败
const qint8 USER_EXISTS = 6;                    //注册用户已存在
const qint8 USERNAME_ERROR = 7;                 //登录用户名错误
const qint8 PASSWORD_ERROR = 8;                 //登录密码错误
const qint8 UPLOAD_REQ = 9;                     //上传文件请求
const qint8 FILE_EXISTS = 10;                   //该名称的文件或者文件夹在待上传位置已经存在
const qint8 CAN_UPLOAD = 11;                    //服务器收到传输文件请求，并且能够接收待上传的文件
const qint8 OVER_LIMIT = 12;                    //文件太大，或者超出用户空间
const qint8 PATH_INCOMPLETE = 13;               //文件路径残缺，当前文件路径不存在，或者缺失
const qint8 SURE_UPLOAD = 14;                   //确认上传文件
const qint8 CANCEL_UPLOAD = 15;                 //取消上传文件
const qint8 DOWNLOAD_REQ = 16;                  //下载文件请求
const qint8 CAN_DOWNLOAD = 17;                  //可以下载
const qint8 SURE_DOWNLOAD = 18;                 //确认下载
const qint8 CANCEL_DOWNLOAD = 19;               //取消下载
const qint8 FILE_NOT_EXISTS = 20;               //文件不存在
const qint8 FILE_LIST_REQ = 21;                 //获取文件列表
const qint8 FILE_LIST_INFO = 22;                //文件列表信息
const qint8 CREATE_FOLDER_REQ = 23;             //创建文件夹请求
const qint8 FOLDER_EXISTS = 24;                 //文件夹已经存在
const qint8 FOLDER_CREATE_SUCCESS = 25;         //文件夹创建成功
const qint8 FILE_TRANSFER_COMPLETE = 26;        //文件传输完成
const qint8 DELETE_FILE_REQ = 27;               //删除文件请求
const qint8 DELETE_FILE_COMPLETE = 28;          //删除文件完成

class FileInfo                  //文件基本信息
{
public:
    FileInfo();

    QString fileName;           //文件名
    qint64  fileSize;           //文件大小
    qint32  fileType;           //文件类型,目前只区分文件夹和普通文件,1表示为文件夹，

    QString filePath;           //文件路径
    QString targetPath;         //待上传文件的目标路径
    QString uploadTime;         //上传时间
};

#endif // CONSTANT_H
