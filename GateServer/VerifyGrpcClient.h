#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

//连接池
class RPConPool {
public:
	RPConPool(size_t poolsize, std::string host, std::string port):
	poolSize_(poolsize), host_(host), port_(port), b_stop_(false) {
		//通过CreateChannel 建立与目标gRPC服务器的底层连接
		//InsecureChannelCredentials 不使用TLS加密
		for (size_t i = 0; i < poolSize_; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
				grpc::InsecureChannelCredentials());
			//基于channel创建业务层Stub，并push进队列（池子）中
			connections_.push(VarifyService::NewStub(channel));
		}

	}

	~RPConPool() {
		//加锁后调用close（）通知所有等待的进程
		std::lock_guard < std::mutex > lock(mutex_);
		close();
		//清空队列
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	void close() {
		b_stop_ = true;
		cond_.notify_all();

	}

	std::unique_ptr<VarifyService::Stub> getConnection() {
		std::unique_lock < std::mutex > lock(mutex_);
		cond_.wait(lock, [this]() {
			if (b_stop_) {
				return true;
			}
			return !connections_.empty();
		});

		if (b_stop_) {
			return nullptr;
		}

		auto context=std::move(connections_.front());
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<VarifyService::Stub> context) {
		std::lock_guard < std::mutex > lock(mutex_);
		if (b_stop_) {
			return;
		}

		connections_.push(std::move(context));
		cond_.notify_one();
	}


private:
	std::atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
	std::condition_variable cond_;
	std::mutex mutex_;
};





//单例客户端
class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyRsp reply;
		GetVarifyReq request;
		request.set_email(email);

		auto stub = pool_->getConnection();
		//发起同步gRPC调用
		Status status = stub->GetVarifyCode(&context, request, &reply);
		if (status.ok()) {
			pool_->returnConnection(std::move(stub));
			return reply;
		}
		else {
			pool_->returnConnection(std::move(stub));
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient();
	std::unique_ptr<RPConPool> pool_;
};

