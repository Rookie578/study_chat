#pragma once
#include "const.h"
#include "MysqlDao.h"
#include "Singleton.h"
class MysqlMgr: public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr();
	int RegUser(const std::string& name, const std::string& email,  const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string & email);
	bool UpdatePwd(const std::string& name, const std::string& email);
	bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);

	bool AddFriendApply(const int& from, const int& to);

	bool AuthFriendApply(const int& from, const int& to);
	bool AddFriend(const int& from, const int& to, std::string back_name);

	std::shared_ptr<UserInfo> GetUser(int uid);

	std::shared_ptr<UserInfo> GetUser(std::string name);

	bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit = 10);

	bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list);

	bool SaveChatMessages(int from_uid, int to_uid, const std::vector<ChatMessageInfo>& msgs);
	bool MarkChatMessages(int to_uid, int from_uid, const std::vector<std::string>& msg_ids, int status, bool all);

private:
	MysqlMgr();
	MysqlDao  _dao;
};

