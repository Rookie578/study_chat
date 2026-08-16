#include "httpmgr.h"

HttpMgr::~HttpMgr()
{

}

HttpMgr::HttpMgr() {
    //一旦HTTP请求完成发出通用信号，立刻触发这个路由槽函数
    connect(this,&HttpMgr::sig_http_finish,this,&HttpMgr::slot_http_finish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    QByteArray data = QJsonDocument(json).toJson();
    /*将JSON对象通过QJsonDocument序列化（打包）成一个QByteArray（字节数组）。
      原因：根据网络通信规则，HTTP协议在网线上传输的只能是二进制字节流，不能传QJsonObject对象。*/
    QNetworkRequest request(url); //创建一个请求对象，并填上服务器的URL
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json"); //告诉服务器传的是JSON格式的数据
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length()));

    //发送异步请求
    //“保命”操作。为了防止在等待期间HttpMgr自己被别人释放，就先获取一个指向自己的智能指针self，抓住自己
    auto self = shared_from_this();
    QNetworkReply * reply = _manager.post(request,data); //发送POST请求

    QObject::connect(reply,&QNetworkReply::finished,[self,reply,req_id,mod](){
        //处理错误情况
        if(reply->error() != QNetworkReply::NoError){
            qDebug()<< reply->errorString();
            //发送信号通知完成
            emit self->sig_http_finish(req_id,"",ErrorCodes::ERR_NETWORK,mod);//发送失败信号
            reply->deleteLater(); //释放内存
            return;
        }

        //无错误情况下
        QString res = reply->readAll();
        //发送信号通知完成
        emit self->sig_http_finish(req_id,res,ErrorCodes::SUCCESS,mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    //判断这个HTTP请求属于哪一个业务模块
    //注册模块
    if(mod==Modules::REGISTERMOD){
        //发送信号通知指定模块http的响应结束
        emit sig_reg_mod_finish(id,res,err);
    }

    //重置模块
    if(mod == Modules::RESETMOD){
        //发送信号通知指定模块http响应结束
        emit sig_reset_mod_finish(id,res,err);
    }

    //登录模块
    if(mod==Modules::LOGINMOD){
        emit sig_login_mod_finish(id,res,err);
    }


















}
