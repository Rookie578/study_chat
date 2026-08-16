#include "CServer.h"
#include "HttpConnection.h"

#include "AsioIOServicePool.h"

//对于类的成员是引用类型，需要在类的初始化列表里进行初始化
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):_ioc(ioc),
_acceptor(ioc,tcp::endpoint(tcp::v4(),port)) {


}

void CServer::Start() {
	auto self = shared_from_this();
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
	_acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
		try {
			//如果出错就放弃该链接，继续监听其他链接
			if (ec) {
				self->Start();
				return;
			}

			
			new_con->Start();


			//继续监听
			self->Start();


		}
		catch (std::exception& exp) {



		}

		

		});

}

