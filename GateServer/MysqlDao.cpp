#include "MysqlDao.h"
#include "ConfigMgr.h"

#include "PasswordHash.h"

MysqlDao::MysqlDao()
{
    auto& cfg = ConfigMgr::Inst();
    const auto& host = cfg["Mysql"]["Host"];
    const auto& port = cfg["Mysql"]["Port"];
    const auto& pwd = cfg["Mysql"]["Passwd"];
    const auto& schema = cfg["Mysql"]["Schema"];
    const auto& user = cfg["Mysql"]["User"];
    // 创建连接池
    pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() {
    pool_->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            
            return false;
        }
        // 准备调用存储过程
        std::unique_ptr < sql::PreparedStatement > stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
        // 设置输入参数
        stmt->setString(1, name);
        stmt->setString(2, email);
        std::string hashed_pwd = PasswordHash::HashPassword(pwd);
        if (hashed_pwd.empty()) {
            std::cerr << "hash password failed" << std::endl;
            pool_->returnConnection(std::move(con));
            return -1;
        }
        stmt->setString(3, hashed_pwd);

        // 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

          // 执行存储过程
        stmt->execute();
        // 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
       // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
        std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
        if (res->next()) {
            int result = res->getInt("result");
            std::cout << "Result: " << result << std::endl;
            pool_->returnConnection(std::move(con));
            return result;
        }
        pool_->returnConnection(std::move(con));
        return -1;
    }
    catch (sql::SQLException& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return -1;
    }
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            //pool_->returnConnection(std::move(con));
            return false;
        }

        // 准备查询语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));

        // 绑定参数
        pstmt->setString(1, name);

        // 执行查询
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // 遍历结果集
        while (res->next()) {
            std::cout << "Check Email: " << res->getString("email") << std::endl;
            if (email != res->getString("email")) {
                pool_->returnConnection(std::move(con));
                return false;
            }
            pool_->returnConnection(std::move(con));
            return true;
        }
        return true;
    }
    catch (sql::SQLException& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd) {
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            //pool_->returnConnection(std::move(con));
            return false;
        }

        // 准备查询语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

        // 绑定参数
        pstmt->setString(2, name);
        std::string hashed = PasswordHash::HashPassword(newpwd);
        if (hashed.empty()) {
            pool_->returnConnection(std::move(con));
            return false;
        }
        pstmt->setString(1, hashed);

        // 执行更新
        int updateCount = pstmt->executeUpdate();

        std::cout << "Updated rows: " << updateCount << std::endl;
        pool_->returnConnection(std::move(con));
        return true;
    }
    catch (sql::SQLException& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MysqlDao::CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo) {
    auto con = pool_->getConnection();
    if (con == nullptr) {
        return false;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
        });

    try {


        // 准备SQL语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
        pstmt->setString(1, email); // 将username替换为你要查询的用户名

        // 执行查询
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::string origin_pwd, row_name, row_email;
        int row_uid = 0;
        if (res->next()) {
            origin_pwd = res->getString("pwd");
            row_name = res->getString("name");
            row_email = res->getString("email");
            row_uid = res->getInt("uid");
        }
        // 先把结果集放掉，再执行 UPDATE，避免 MySQL 报 commands out of sync
        res.reset();

        if (origin_pwd.empty()) {
            return false;                 // 邮箱不存在
        }

        if (PasswordHash::IsHashed(origin_pwd)) {
            if (!PasswordHash::VerifyPassword(pwd, origin_pwd)) {
                return false;
            }
        }
        else {
            // 旧格式（异或值）：兼容校验一次，成功就顺手升级成 PBKDF2
            if (!PasswordHash::ConstantTimeStringEqual(
                PasswordHash::LegacyXor(pwd), origin_pwd)) {
                return false;
            }
            std::string upgraded = PasswordHash::HashPassword(pwd);
            if (!upgraded.empty()) {
                try {
                    std::unique_ptr<sql::PreparedStatement> up(
                        con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE email = ?"));
                    up->setString(1, upgraded);
                    up->setString(2, email);
                    up->executeUpdate();
                    std::cout << "[migrate] legacy password upgraded: " << email << std::endl;
                }
                catch (sql::SQLException& e) {
                    std::cerr << "upgrade pwd failed: " << e.what() << std::endl;
                }
            }
        }

        userInfo.name = row_name;
        userInfo.email = row_email;
        userInfo.uid = row_uid;
        return true;
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }

}