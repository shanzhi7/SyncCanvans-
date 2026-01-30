#include "CanvasServer/CServer.h"
#include "CanvasServer/CSession.h"

CServer::CServer(boost::asio::io_context& ioc, short port)
	:_io_context(ioc), _port(port),_acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	std::cout << "[CServer] Server initialized on port: " << _port << std::endl;
	Start();	// Start
}
CServer::~CServer()
{
	std::cout << "[CServer] Server stopped." << std::endl;
}
void CServer::Start()
{
	StartAccept();	//开始监听新连接
}
void CServer::StartAccept()
{ 
	auto& worker_io_context = AsioIOServicePool::getInstance()->GetIOService();	// 获取io_context
	// 创建一个Session
	std::shared_ptr<CSession> new_session = std::make_shared<CSession>(worker_io_context);

	//异步接受新连接
	_acceptor.async_accept(new_session->GetSocket(),
		[this, new_session](const boost::system::error_code& error)
		{
			HandleAccept(new_session, error);
		});

}
void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
	if (!error)
	{
		// 启动Session
		new_session->Start();
	}
	else
	{
		std::cout << "[CServer] Accept error: " << error.message() << std::endl;
	}

	// 继续监听新连接
	StartAccept();
}