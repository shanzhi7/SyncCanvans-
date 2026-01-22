// LogicServer.cpp: 定义应用程序的入口点。
//

#include "LogicServer/LogicServer.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <grpcpp/grpcpp.h>

#include "LogicServer/LogicServiceImpl.h"
#include "LogicServer/ConfigMgr.h"
#include "LogicServer/RedisMgr.h"
#include "LogicServer/MysqlMgr.h" // 假设你封装了 MysqlMgr 单例

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// 启动grpc服务函数
void RunServer()
{
    // 读取配置文件
    auto& cfg = ConfigMgr::Inst();
    std::string host = cfg["LogicServer"]["Host"];
    std::string port = cfg["LogicServer"]["Port"];
    std::string server_address = host + ":" + port;

    // 创建服务
    LogicServiceImpl service;               // 实例化服务类
    ServerBuilder builder;                  // 创建服务构建器
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());    // 监听端口 (不使用 SSL 证书)
    builder.RegisterService(&service);      // 注册服务

    // 启动服务
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "[LogicServer] gRPC Server listening on " << server_address << std::endl;

    //阻塞等待，直到服务器被关闭
    server->Wait();
}

int main()
{
    try
    {
        //初始化各种单例类，确保它们在程序运行期间可用
        ConfigMgr::Inst();
        std::cout << "[LogicServer] Config loaded." << std::endl;
        MysqlMgr::getInstance();
        std::cout << "[LogicServer] MysqlMgr initialized." << std::endl;
        RedisMgr::getInstance();
        std::cout << "[LogicServer] RedisMgr initialized." << std::endl;

        // 启动grpc服务
        RunServer();
    }
    catch (const std::exception& e)
    {
        std::cerr << "[LogicServer] Crashed with exception: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "[LogicServer] Crashed with unknown exception." << std::endl;
        return -1;
    }
	return 0;
}
