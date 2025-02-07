/*
muduo网络库给客户提供两个主要的类
TcpServer:用于编写服务器程序的
TcpClient:用于编写客户端程序的

epoll + 线程池
好处：能把网络I/O的代码和业务代码区分开
                       用户的链接和断开  用户的可读写事件
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional> //内含绑定器
#include <string>

using namespace std;
using namespace placeholders; // 参数占位符

// 基于muduo网络库开发服务器程序
// 1.组合TcpServer对象
// 2.创建EventLoop事件循环对象的指针
// 3.明确TcpServer构造函数需要什么参数，输出ChatServer
// 4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
// 5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop *loop,               // 事件循环
               const muduo::net::InetAddress &listenAddr, // Ip+Port
               const string &nameArg)                     // 服务器名字
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        // 原函数参数是另一个函数的引用，该函数参数(指另一个函数)是const muduo::net::TcpConnectionPtr&
        // 这里用bind实现原参数的功能，bind作为成员函数第二参数为调用第一参数的对象或对象的指针，这里即ChatServer类自己，第三参数是函数(第一参数)的参数的占位符

        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 设置服务器端的线程数量 1个I/O线程 3个worker线程
        _server.setThreadNum(4);
    }

    // 开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开 epoll listenfd accept
    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        // 这个函数是实现muduo::net::ConnectionCallback的功能，
        // 他是std::function<void (const muduo::net::TcpConnectionPtr &)>的别名，
        // 它应该是用来创建一个存储void (const muduo::net::TcpConnectionPtr &)函数的模板函数，大概是吧，我也不太明白，2025/1/21

        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " start:online " << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " start:offline " << endl;
            conn->shutdown();//close(fd)
            //_loop->quit();
        }
    }

    // 专门处理用户读写事件
    void onMessage(const muduo::net::TcpConnectionPtr &conn, // 连接
                   muduo::net::Buffer *buffer,               // 缓冲区
                   muduo::Timestamp time)                         // 接收到数据的时间信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << " time:" << time.toString() << endl;
        conn->send(buf);
    }

    muduo::net::TcpServer _server; // #1
    muduo::net::EventLoop *_loop;  // #2 epoll
};

int main()
{
    muduo::net::EventLoop loop;//epoll
    muduo::net::InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");

    server.start();//listenfd epoll_ctl=>epoll
    loop.loop();//epoll_wait以阻塞的方式等待新用户连接、已连接用户的读写事件等

    return 0;
}