#pragma once
#include "LogicServer/MysqlPool.h"

class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();

	//зЂВс
	int Register(const std::string& name, const std::string& email, const std::string& password,
		int sex = 0, const std::string& avatar = "", const std::string& signature = "");

private:
	std::unique_ptr<MysqlPool> _mysqlPool;
};