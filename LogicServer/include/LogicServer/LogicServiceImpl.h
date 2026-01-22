#pragma once
#include <grpcpp/grpcpp.h>
#include "LogicServer/message.grpc.pb.h"
#include "LogicServer/const.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::RegisterReq;
using message::RegisterRsp;
using message::LogicService;

class LogicServiceImpl final : public LogicService::Service
{
public:
    LogicServiceImpl() = default;

    // 重写 .proto 中定义的 RegisterUser 方法
    virtual Status RegisterUser(ServerContext* context, const RegisterReq* request, RegisterRsp* reply) override;
};