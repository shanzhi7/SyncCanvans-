#pragma once
#include "LogicServer/MysqlPool.h"
#include "LogicServer/data.h"

class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();

	//×¢²á
	int Register(const std::string& name, const std::string& email, const std::string& password,
		int sex = 0, const std::string& avatar = "", const std::string& signature = "");

	//ÖØÖÃÃÜÂë
	int ResetPassword(const std::string& email, const std::string& verifycode, const std::string& password);

	//Ğ£ÑéÃÜÂë
	bool CheckPassword(const std::string& email, const std::string& pwd, UserInfo& userInfo);



private:
	std::unique_ptr<MysqlPool> _mysqlPool;
};