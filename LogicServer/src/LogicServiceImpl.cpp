#include "LogicServer/LogicServiceImpl.h"
#include "LogicServer/RedisMgr.h"
#include "LogicServer/MysqlMgr.h"
#include <iostream>

// 注册用户函数
Status LogicServiceImpl::RegisterUser(ServerContext* context, const RegisterReq* request, RegisterRsp* reply)
{
	//获取参数
    std::string name = request->name();
    std::string email = request->email();
    std::string passwd = request->passwd();
    std::string confirm_pwd = request->confirm_pwd();
    std::string varifycode = request->varifycode();

    std::cout << "[LogicServer] 收到注册请求: email=" << email << ", name=" << name << std::endl;

    //验证验证码
    std::string redis_key = CODEPREFIX + email;
    std::string redis_code;
    // 从redis中获取验证码
    bool get_success = RedisMgr::getInstance()->Get(redis_key, redis_code);
    if (!get_success)
    {
        std::cout<<"[LogicServer] 验证码已过期或不存在"<<std::endl;
        reply->set_error(message::ErrorCodes::VarifyExpired);
        return Status::OK;
    }
    if (redis_code != varifycode)
    {
        std::cout << "[LogicServer] 验证码错误: " << email <<std::endl;
        reply->set_error(message::ErrorCodes::VarifyCodeErr);
        return Status::OK;
    }

    //成功，写入mysql
    int result = MysqlMgr::getInstance()->Register(name, email, passwd);
    if (result == message::ErrorCodes::UserExist)
    {
        // 用户名/邮箱已存在
        std::cout << "[LogicServer] 注册失败，用户已存在: " << email << std::endl;
        reply->set_error(message::ErrorCodes::UserExist);
        return Status::OK;
    }
    if (result <= 0)
    {
        // 数据库内部错误 (连接失败、SQL语法错等)
        std::cout << "[LogicServer] 注册失败，数据库内部错误" << std::endl;
        reply->set_error(message::ErrorCodes::RPCFailed); // 或者定义一个 DBError
        return Status::OK;
    }

    // 注册成功
    RedisMgr::getInstance()->Del(redis_key);    // 删除验证码
    std::cout << "[LogicServer] 注册成功: " << email << ", uid=" << result << std::endl;
    reply->set_error(message::ErrorCodes::SUCCESS);
    reply->set_uid(result); // 将生成的 UID 返回给前端

    return Status::OK;
}

Status LogicServiceImpl::ResetPassword(ServerContext* context, const ResetPasswordReq* request, ResetPasswordRsp* reply)
{ 
    // 获取参数
    std::string email = request->email();
    std::string varifycode = request->varifycode();
    std::string passwd = request->passwd();
    std::string confirm_pwd = request->confirm_pwd();
    std::cout << "[LogicServer] 密码重置请求: email=" << email << std::endl;
    if (passwd != confirm_pwd)
    {
        std::cout << "[LogicServer] 密码不一致" << std::endl;
        reply->set_error(message::ErrorCodes::PasswdInvalid);
        return Status::OK;
    }
    
    // 验证验证码
    std::string redis_key = CODEPREFIX + email;
    std::string redis_code;
    bool get_success = RedisMgr::getInstance()->Get(redis_key, redis_code);
    if (!get_success)
    {
        std::cout << "[LogicServer] 验证码已过期或不存在" << std::endl;
        reply->set_error(message::ErrorCodes::VarifyExpired);
        return Status::OK;
    }
    if (redis_code != varifycode)
    {
        std::cout << "[LogicServer] 验证码错误: " << email << std::endl;
        reply->set_error(message::ErrorCodes::VarifyCodeErr);
        return Status::OK;
    }
    int result = MysqlMgr::getInstance()->ResetPassword(email, varifycode, passwd); // 调用 MysqlMgr
    if (result != message::ErrorCodes::SUCCESS)
    {
        std::cout << "[LogicServer] 密码重置失败，数据库内部错误" << std::endl;
        reply->set_error(message::ErrorCodes::RPCFailed);
        return Status::OK;
    }

    RedisMgr::getInstance()->Del(redis_key);    // 删除验证码

    std::cout << "[LogicServer] 密码重置成功: " << email << std::endl;
    return Status::OK;
}