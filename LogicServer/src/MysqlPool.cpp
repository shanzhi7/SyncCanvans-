#include"LogicServer/MysqlPool.h"

MysqlPool::MysqlPool(std::string url, std::string user, std::string password, std::string schema, int poolSize)
:
    _url(std::move(url)),
    _user(std::move(user)),
    _password(std::move(password)),
    _schema(std::move(schema)),
    _poolSize(poolSize),
    _isClosed(false)
{
    // 初始化连接池
    try
    {
        for (int i = 0; i < _poolSize; ++i)
        {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();           // 获取MySQL驱动实例
            std::unique_ptr<sql::Connection> conn(driver->connect(_url, _user, _password)); // 创建连接
            conn->setSchema(_schema);                                                       // 设置数据库

            // 将连接放入连接池
            _pool.push(std::move(conn));
        }
    }
    catch(sql::SQLException const& e)
    {
        std::cout << "数据库连接池初始化失败: " << e.what() << std::endl;
        std::cout << e.what() << std::endl;
    }
}
MysqlPool::~MysqlPool()
{
    std::lock_guard<std::mutex> lock(_mutex);   // 加锁
    // 销毁连接池
    while (!_pool.empty())
    {
        _pool.pop();
    }
}
void MysqlPool::close()
{
    _isClosed = true;
    _cond.notify_all();     // 通知所有线程
}

std::unique_ptr<sql::Connection> MysqlPool::getConnection()
{
    std::unique_lock<std::mutex> lock(_mutex);      // 加锁
    _cond.wait(lock, [this] {                       // 等待连接池非空
        if (_isClosed)
        {
            return true;
        }
        return !_pool.empty(); 
        });
    if (_isClosed)
    {
        return nullptr;
    }
    std::unique_ptr<sql::Connection> conn(std::move(_pool.front()));
    _pool.pop();
    return conn;
}
void MysqlPool::returnConnection(std::unique_ptr<sql::Connection> conn)
{
    std::lock_guard<std::mutex> lock(_mutex);       // 加锁
    if (_isClosed)
        return;
    _pool.push(std::move(conn));                    // 将连接放回连接池
    _cond.notify_one();                             // 通知一个线程
}