// CanvasServer.cpp: 定义应用程序的入口点。
//

#include "CanvasServer/CanvasServer.h"
#include "CanvasServer/CServer.h"
#include "CanvasServer/ConfigMgr.h"
#include "CanvasServer/RedisMgr.h"
#include "CanvasServer/LogicSystem.h"
#include "CanvasServer/SessionMgr.h"
#include "CanvasServer/RoomMgr.h"
#include "CanvasServer/AsioIOServicePool.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>

using namespace std;

int main()
{
    try
    {
        // 初始化配置
        auto& cfg = ConfigMgr::Inst();
        std::cout << "[CanvasServer] Config loaded." << std::endl;

        // 初始化 Redis
        RedisMgr::getInstance();
        std::cout << "[CanvasServer] RedisMgr initialized." << std::endl;

        // 初始化线程池 (AsioIOServicePool)
        // 要在 Server 启动前把 IO 线程跑起来
        AsioIOServicePool::getInstance();
        std::cout << "[CanvasServer] IO Thread Pool initialized." << std::endl;

        // 初始化业务逻辑系统 (LogicSystem 构造时会启动业务处理线程)
        LogicSystem::getInstance();
        std::cout << "[CanvasServer] LogicSystem initialized (Worker thread started)." << std::endl;

        // 初始化管理器
        SessionMgr::getInstance();
        RoomMgr::getInstance();
        std::cout << "[CanvasServer] SessionMgr & RoomMgr initialized." << std::endl;

        // 准备网络环境
        std::string host = cfg["SelfServer"]["Host"];
        int port = std::stoi(cfg["SelfServer"]["Port"]);

        // 创建主线程的 io_context，专门给 CServer 的 Acceptor 用
        boost::asio::io_context io_context;

        // 注册信号处理 (Ctrl+C 优雅退出)
        // 防止强制关闭导致单例析构异常
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](const boost::system::error_code& error, int signal_number)
            {
                if (!error)
                {
                    std::cout << "[CanvasServer] Catch signal " << signal_number << ", stopping..." << std::endl;

                    // 停止 Accept
                    io_context.stop();

                    // 停止 IO 线程池
                    AsioIOServicePool::getInstance()->Stop();
                }
            });

        // 启动 TCP 服务器
        // CServer 构造函数里已经写了 StartAccept()，所以实例化就会开始监听
        CServer server(io_context, port);
        std::cout << "[CanvasServer] TCP Server listening on port " << port << std::endl;

        // 阻塞主线程，处理连接请求
        // 之后的 Read/Write 操作会由 AsioIOServicePool 里的线程去跑，不占用这里
        io_context.run();

        std::cout << "[CanvasServer] Server stopped successfully." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[CanvasServer] Crashed with exception: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "[CanvasServer] Crashed with unknown exception." << std::endl;
        return -1;
    }
	return 0;
}
