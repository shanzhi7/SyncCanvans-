#pragma once
#include <jdbc/mysql_driver.h>					// 包含 MySQL_Driver 类（驱动实例）
#include <jdbc//mysql_connection.h>				// 包含 Connection 类（数据库连接）
#include <jdbc/cppconn/prepared_statement.h>	// PreparedStatement（如存储过程调用）
#include <jdbc//cppconn//exception.h>			// 包含 sql::SQLException（MySQL 异常类）
#include <string>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

class MysqlPool
{
public:
    MysqlPool(std::string url, std::string user, std::string password, std::string schema, int poolSize);
    ~MysqlPool();

    std::unique_ptr<sql::Connection> getConnection();               // 获取连接
    void returnConnection(std::unique_ptr<sql::Connection> con);    // 归还连接
    void close();                                                   //关闭连接池

private:
	std::string _url;			// 数据库连接地址
    std::string _user;			// 用户名
    std::string _password;      // 密码
    std::string _schema;        // 数据库名
    int _poolSize;               // 连接池大小
    std::queue<std::unique_ptr<sql::Connection>> _pool;     // 连接对象队列

    //线程控制
    std::mutex _mutex;                      // 互斥锁
    std::condition_variable _cond;          // 条件变量
    bool _isClosed;                         // 连接池是否关闭
};
