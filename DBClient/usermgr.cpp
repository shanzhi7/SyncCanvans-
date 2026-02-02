#include "usermgr.h"

UserMgr::UserMgr()
    :token(""),_my_info(nullptr)
{

}

QString UserMgr::getToken()
{
    return this->token;
}

void UserMgr::setToken(QString &token)
{
    this->token = token;
}

void UserMgr::setMyInfo(std::shared_ptr<UserInfo> userInfo)
{
    this->_my_info = userInfo;
}

std::shared_ptr<UserInfo> UserMgr::getMyInfo()
{
    return this->_my_info;
}
