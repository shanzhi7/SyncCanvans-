#include "CanvasServer/LogicSystem.h"
#include "CanvasServer/SessionMgr.h"
#include "CanvasServer/RoomMgr.h"
#include "CanvasServer/RedisMgr.h"
#include "CanvasServer/const.h"      // 包含 MSG_IDS 定义
#include "CanvasServer/message.pb.h"
#include <iostream>

LogicSystem::LogicSystem() : _b_stop(false)
{
	RegisterCallBacks();	// 注册回调函数
	_work_thread = std::thread(&LogicSystem::DealMsg, this);	//启动工作线程
}

LogicSystem::~LogicSystem()
{
    _b_stop = true;
    _cond.notify_one();
    if (_work_thread.joinable())
    {
        _work_thread.join();
    }
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
    // 生产者写法
    std::unique_lock<std::mutex> lock(_mutex);
    _msg_queue.push(msg);
    if (_msg_queue.size() == 1)
    {
        lock.unlock(); // 提早解锁
        _cond.notify_one();
    }
}

void LogicSystem::DealMsg()
{
    while (true)    // 死循环,处理需要发送的消息
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock, [this](){
            return _b_stop || !_msg_queue.empty();
            });

        if (_b_stop && _msg_queue.empty())
        {
            break;
        }

        //取出头部消息
        auto msg_node = _msg_queue.front();
        _msg_queue.pop();
        lock.unlock();  // 处理消息时释放锁，允许新消息入队

        //查找回调函数
        auto it = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
        if (it != _fun_callbacks.end())
        {
            // 执行业务逻辑
            // RecvNode 里的 _data 是 char*，转 string 传给业务层方便解析 Json
            it->second(msg_node->_session,
                        msg_node->_recvnode->_msg_id,
                        std::string(msg_node->_recvnode->_data,msg_node->_recvnode->_cur_len));
        }
        else
        {
            std::cout << "【LogicSystem】: Unknown msg id " << msg_node->_recvnode->_msg_id << std::endl;
        }
    }
}

void LogicSystem::RegisterCallBacks()
{
    // 注册登录
    _fun_callbacks[ID_CANVAS_LOGIN_REQ] = std::bind(&LogicSystem::HandleLogin, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    // 注册进房
    _fun_callbacks[ID_JOIN_ROOM_REQ] = std::bind(&LogicSystem::HandleJoinRoom, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    //不需要注册 ID_DRAW_REQ (画画请求)
    // 画画请求在 CSession 层直接被拦截转发了，不会进这个队列
}

void LogicSystem::HandleLogin(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;

    rsp["error"] = message::ErrorCodes::Error_Json; // 默认错误码

    //作用域结束发送回包
    Defer defer([this, session, &rsp]() {
        std::string return_str = rsp.toStyledString();
        session->Send(return_str, ID_CANVAS_LOGIN_RSP);

        // 如果是严重错误（如认证失败），发完包后可以考虑打印日志
        int err = rsp["error"].asInt();
        if (err != message::ErrorCodes::SUCCESS) {
            std::cout << "[CanvasServer] Login Error Response sent. Code: " << err << std::endl;
        }
        });


    if (!reader.parse(msg_data, root))
    {
        // 解析失败，直接 return
        // 此时会触发 Defer，发送 Error_Json 给客户端
        std::cout << "[CanvasServer] Login parse json failed." << std::endl;
        return;
    }
    if (!root.isMember("uid") || !root.isMember("token"))   //检测字段是否存在
    {
        rsp["error"] = message::ErrorCodes::Error_Json;
        std::cout << "[CanvasServer] Login Error: Missing field." << std::endl;
        return;
    }

    int client_uid = root["uid"].asInt();
    std::string client_token = root["token"].asString();

    //构造redis key
    std::string token_key = TOKEN_PREFIX + client_token;

    //查找redis，有没有该token对应的uid
    std::string valid_uid_str;
    bool b_exist = RedisMgr::getInstance()->Get(token_key, valid_uid_str);

    //token过期或者错误
    if (!b_exist)
    {
        rsp["error"] = message::ErrorCodes::TokenInvalid;
        std::cout << "[CanvasServer] Token not found in Redis. Client UID: " << client_uid << std::endl;
        return; //触发Defer发送 TokenInvalid
    }

    //token存在，但是uid是别人的
    if (std::stoi(valid_uid_str) != client_uid)
    {
        rsp["error"] = message::ErrorCodes::TokenInvalid;
        std::cout << "[CanvasServer] Token mismatch! Redis UID: " << valid_uid_str << " Client UID: " << client_uid << std::endl;
        return; // 触发 Defer 发送 TokenInvalid
    }

    // 校验成功！
    session->SetUserId(client_uid);     //绑定UID到Session

    SessionMgr::getInstance()->AddSession(client_uid,session);  //注册到全局管理器(SessionMgr)

    // 构造成功回包
    rsp["error"] = message::ErrorCodes::SUCCESS;
    rsp["uid"] = client_uid;

    std::cout << "[CanvasServer] User Login Success: " << client_uid << std::endl;

    // 函数结束，Defer 自动析构，发送 SUCCESS 包
}

void LogicSystem::HandleJoinRoom(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
    Json::Reader reader;
    Json::Value root;
    Json::Value rsp;

    //todo...

    //reader.parse(msg_data, root);
    //std::string room_id = root["room_id"].asString();
    //int uid = session->GetUserId();

    //// 1. 获取或创建房间
    //// RoomMgr 是单例，管理所有 Room
    //auto room = RoomMgr::GetInstance()->GetOrCreateRoom(room_id);

    //if (!room) {
    //    rsp["error"] = 1; // Failed
    //    session->Send(rsp.toStyledString(), ID_JOIN_ROOM_RSP);
    //    return;
    //}

    //// 2. 将 Session 加入房间
    //// Join 内部会广播 "User XXX Joined" 给房间其他人
    //// 并且会把历史笔迹发给当前 session
    //room->Join(session);

    //// 3. 回复客户端：加入成功
    //rsp["error"] = 0;
    //rsp["room_id"] = room_id;
    //session->Send(rsp.toStyledString(), ID_JOIN_ROOM_RSP);

    //std::cout << "User " << uid << " joined room " << room_id << std::endl;
}