#include "usermgr.h"

UserMgr::UserMgr() {}

std::shared_ptr<UserInfo> UserMgr::getMyInfo()
{
    return this->_my_info;
}
