#include "LogicServer/MysqlMgr.h"

MysqlMgr::MysqlMgr()
{
    
}
MysqlMgr::~MysqlMgr()
{
    
}
// зЂВс
int MysqlMgr::Register(const std::string& name, const std::string& email, const std::string& password,
    int sex, const std::string& avatar, const std::string& signature)
{
    return _dao.Register(name, email, password, sex, avatar, signature);
}