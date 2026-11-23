#include "UserMgr.h"
#include "CSession.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"

UserMgr:: ~UserMgr() {
	_uid_to_session.clear();
}


std::shared_ptr<CSession> UserMgr::GetSession(int uid)
{
	std::lock_guard<std::mutex> lock(_session_mtx);
	auto iter = _uid_to_session.find(uid);
	if (iter == _uid_to_session.end()) {
		return nullptr;
	}

	return iter->second;
}

void UserMgr::SetUserSession(int uid, std::shared_ptr<CSession> session)
{
	std::lock_guard<std::mutex> lock(_session_mtx);
	_uid_to_session[uid] = session;
}

void UserMgr::RmvUserSession(int uid, std::shared_ptr<CSession> session)
{
	if (uid <= 0) {
		return;
	}

	std::lock_guard<std::mutex> lock(_session_mtx);
	auto iter = _uid_to_session.find(uid);
	// Only the session currently bound to this uid may remove it. If the user
	// logged in again, the map already points at the new session.
	if (iter == _uid_to_session.end() || iter->second != session) {
		return;
	}
	_uid_to_session.erase(iter);

	// Atomic compare-and-delete: drop the routing key only when it still
	// points at this node. Holding the lock keeps a concurrent same-node
	// re-login from being deleted by this stale session.
	auto server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	RedisMgr::GetInstance()->CompareAndDelete(
		USERIPPREFIX + std::to_string(uid), server_name);
}

UserMgr::UserMgr()
{

}
