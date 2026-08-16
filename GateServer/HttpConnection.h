#pragma once

#include "const.h"


//负责管理单个HTTP连接

class HttpConnection :public std::enable_shared_from_this<HttpConnection>
{
public:
	friend class LogicSystem;
	HttpConnection(boost::asio::io_context & ioc);
	void Start();
	tcp::socket& GetSocket() {
		return _socket;
	}
private:
	void CheckDeadline();
	void WriteResponse();
	void HandleReq();
	void PreParseGetParam();
	tcp::socket _socket; //和客户端（浏览器）之间的专用通道，数据的收发都通过它
	beast::flat_buffer _buffer{ 8192 };   //用来接收数据
	http::request<http::dynamic_body> _request;  //用来解析请求
	http::response<http::dynamic_body> _response; //用来回应客户端
	//用来做定时器判断请求是否超时
	net::steady_timer deadline_{
		_socket.get_executor(),std::chrono::seconds(60)
	};

	//get请求的参数解析
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;

};

