#include "db.h"
#include <muduo/base/Logging.h>

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr); // 开辟一段空间，准备用作连接，并非真的连接，可以传入一个MYSQL指针
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
// 连接数据库
bool MySQL::connect()
{
    // 倒数第二个参数用于指定套接字路径，null表示不使用套接字连接
    // 最后一参数为客户端标志，用于指定一些连接的特性或选项
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), // 真正的连接
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);

    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置，从MySQL上拉下来的中文显示？
        mysql_query(_conn, "set names gbk"); // 向MySQL数据库服务器发送一条sql语句（以字符串形式表示）并执行
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "conect mysql fail!";
    }

    return p;//指针可以被隐式转换成bool类型，null为false，其他为true
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        // LOG_INFO可以保存日志信息
        //__FILE__是一个预定义的宏，是一个字符串常量，其值为当前源文件的文件名（包含完整的路径）
        LOG_INFO << __FILE__ << ":" << sql << "更新失败！";
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << sql << "查询失败！";
        return nullptr;
    }
    // 当使用mysql_query函数执行select语句后，可以使用mysql_use_result函数来初始化结果集的检索，以用于后续操作
    return mysql_use_result(_conn);
}

// 获取连接
MYSQL *MySQL::getConnecction()
{
    return _conn;
}