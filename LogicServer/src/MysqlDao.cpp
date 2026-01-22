#include "LogicServer/MysqlDao.h"
#include "LogicServer/ConfigMgr.h"
#include "LogicServer/message.pb.h"

MysqlDao::MysqlDao()
{
	auto& cfg = ConfigMgr::Inst();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Password"];
	const auto& user = cfg["Mysql"]["User"];
	const auto& schema = cfg["Mysql"]["Schema"];
	std::cout << "读取到的Mysql Host: " << host << std::endl;
	std::cout << "读取到的Mysql port: " << port << std::endl;
	std::cout << "读取到的Mysql password: " << pwd << std::endl;
	std::cout << "读取到的Mysql user: " << user << std::endl;
	std::cout << "读取到的Mysql schema: " << schema << std::endl;
	_mysqlPool.reset(new MysqlPool("tcp://" + host + ":" + port, user, pwd, schema, 5));	// 5为连接池大小
}

MysqlDao::~MysqlDao()
{

}

//注册
int MysqlDao::Register(const std::string& name, const std::string& email, const std::string& password,
	int sex, const std::string& avatar, const std::string& signature)
{
	try
	{
		auto con = _mysqlPool->getConnection();
		if (!con)
		{
			std::cout << "Register Error: 获取连接失败" << std::endl;
			return -1;
		}
		// defer
		Defer defer([&]() {
			_mysqlPool->returnConnection(std::move(con)); 
			});

		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->prepareStatement(
				"INSERT INTO user(name, email, passwd, sex, avatar, signature) VALUES (?, ?, ?, ?, ?, ?)"
			)
		);

		// 绑定 6 个参数
		pstmt->setString(1, name);
		pstmt->setString(2, email);
		pstmt->setString(3, password);
		pstmt->setInt(4, sex);
		pstmt->setString(5, avatar);
		pstmt->setString(6, signature);

		// 执行
		pstmt->executeUpdate();

		//获取自增id
		std::unique_ptr<sql::Statement> stmt(con->createStatement());	// 创建Statement
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID()"));	// 执行查询
		if (res->next())
        {
            int uid = res->getInt(1);
            std::cout << "Register Success: uid = " << uid << std::endl;
            return uid;
        }
		std::cout << "Register Error: 获取自增id失败" << std::endl;
		return -1;
	}
	catch (sql::SQLException& e)
	{

		if (e.getErrorCode() == 1062)// 1062 是 MySQL Duplicate entry(重复条目) 的错误码
		{
			std::cout << "Register Error: 邮箱已存在 (Duplicate entry)" << std::endl;
			return message::ErrorCodes::UserExist;
		}
		std::cerr << "SQLException: " << e.what();
		std::cerr << "(MYSQL error code: )" << e.getErrorCode();
		std::cerr << ",SQLState: " << e.getSQLState() << ")" << std::endl;
		return -1;
	}
}