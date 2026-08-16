#pragma once
#include "const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)>HttpHandler;
class LogicSystem : public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem(){}
	bool HandleGet(std::string,std::shared_ptr<HttpConnection>);  //处理GET请求
	void RegGet(std::string, HttpHandler handler); //注册GET处理器
	void RegPost(std::string url, HttpHandler handler);
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>); //处理post请求
private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers; 
	std::map<std::string, HttpHandler> _get_handlers;  
};