#include "CanvasServer/Room.h"
#include "CanvasServer/CSession.h"
#include "CanvasServer/const.h"

Room::Room(const std::string& room_id)
    : _room_id(room_id)
{
    std::cout << "[Room] Created: " << _room_id << std::endl;
}

Room::~Room()
{
    std::cout << "[Room] Destroyed: " << _room_id << std::endl;
    // 可以在这里清空 _sessions，但智能指针会自动处理
}

void Room::Join(std::shared_ptr<CSession> session)
{
    std::lock_guard<std::mutex> lock(_mutex);

    int uid = session->GetUserId();
    if (uid == 0) // 未登录用户不允许加入
    {
        return;
    }

    _sessions[uid] = session;
    session->SetRoom(shared_from_this());   //这样 Session 断开时知道通知哪个房间,session有room的弱指针

    std::cout << "[Room " << _room_id << "] User joined: " << uid
        << ". Total: " << _sessions.size() << std::endl;

    //todo...
    // 让他看到之前别人画的东西
    //if (!_history.empty())
    //{
    //    // 这里的 ID_DRAW_REQ 假设是画画的数据包 ID
    //    // 在实际业务中，可能需要把多条历史打包成一个包发送，这里简单起见逐条发
    //    for (const auto& draw_data : _history)
    //    {
    //        session->Send(draw_data, ID_DRAW_REQ);
    //    }
    //}
}
void Room::Leave(int uid)
{
    std::lock_guard<std::mutex> lock(_mutex);

    //移除用户
    auto it = _sessions.find(uid);
    if (it != _sessions.end())
    {
        _sessions.erase(it);
        std::cout << "[Room " << _room_id << "] User left: " << uid
            << ". Total: " << _sessions.size() << std::endl;

        //todo...
        //通知其他用户，更新客户端ui
    }
}

void Room::Broadcast(const std::string& data, int msg_id, int exclude_uid)
{ 
    std::lock_guard<std::mutex> lock(_mutex);

    // 如果是画画请求，存入历史记录
    if (msg_id == ID_DRAW_REQ)
    {
        _history.push_back(data);
    }

    // 遍历发送
    for (auto& pair : _sessions)
    {
        int target_uid = pair.first;
        auto target_session = pair.second;

        // 跳过发送者自己 (因为他本地已经画出来了，不需要服务器回显)
        if (target_uid == exclude_uid) continue;

        // 发送
        target_session->Send(data, msg_id);
    }
}