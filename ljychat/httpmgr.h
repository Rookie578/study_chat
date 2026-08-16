#ifndef HTTPMGR_H
#define HTTPMGR_H

#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager> //QT中专门用来发送网络请求的类
#include <QJsonObject>
#include <QJsonDocument>

//定义全局唯一的HTTP请求管理器 HttpMgr
//多重继承和 CRTP奇异递归模板模式public Singleton<HttpMgr>
class HttpMgr:public QObject, public Singleton<HttpMgr>,public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();
    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);

private:
    friend class Singleton<HttpMgr>; //这里必须把 Singleton<HttpMgr> 设为友元（friend），相当于给它发了一张“特权通行证”，允许它去调用 HttpMgr 的私有构造函数来创建那个唯一的实例。
    HttpMgr();
    QNetworkAccessManager _manager;


private slots:
    void slot_http_finish(ReqId id,QString res,ErrorCodes err,Modules mod);

signals:
    void sig_http_finish(ReqId id,QString res,ErrorCodes err,Modules mod);

    void sig_reg_mod_finish(ReqId id,QString res,ErrorCodes err);

    void sig_reset_mod_finish(ReqId id,QString res,ErrorCodes err);

    void sig_login_mod_finish(ReqId id,QString res,ErrorCodes err);
};

#endif // HTTPMGR_H
