#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono> //时间相关，高精度
#include <ctime>  //时间相关，较为基本
using namespace std;
using json = nlohmann::json;

// Unix/Linux 系统下网络编程的基础
#include <unistd.h>     //系统调用（文件、进程、套接字操作）。
#include <sys/socket.h> //套接字编程核心功能。
#include <sys/types.h>  //辅助数据类型定义。
#include <netinet/in.h> //IPv4/IPv6 地址结构和字节序转换。
#include <arpa/inet.h>  //IP 地址字符串与二进制的互转。

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;
// 显示当前登录成功用户的基本信息
void showCurrentUserData();
// 控制主菜单页面
bool isMainMenuRunning = false;

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int clientfd);

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接受线程
// 现在有个问题，一次登录两个号，记录会叠加，我最开始还以为sql有问题
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl; // cerr标准错误输出流，无缓冲，即时 clog有缓冲高性能
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建client端的socket
    /*
    socket函数
    __domain 指定套接字使用的协议族（Protocol Family），常见选项：
                                        AF_INET：IPv4 协议（最常用）。
                                        AF_INET6：IPv6 协议。
                                        AF_UNIX 或 AF_LOCAL：本地进程间通信（Unix 域套接字）。
    __type  指定套接字的 通信类型，常见选项：
                                    SOCK_STREAM：面向连接的流式套接字（如 TCP，可靠传输）。
                                    SOCK_DGRAM：无连接的数据报套接字（如 UDP，快速但不可靠）。
                                    SOCK_RAW：原始套接字（允许直接操作网络层协议，需管理员权限）。
    __protocol  指定具体的传输协议，通常设为 0，表示根据 domain 和 type 自动选择默认协议。例如：
                               SOCK_STREAM 默认使用 TCP。
                               SOCK_DGRAM 默认使用 UDP。
                               手动指定协议时，可用 IPPROTO_TCP 或 IPPROTO_UDP。
    返回值 成功：返回一个 Socket 文件描述符（非负整数）。   失败：返回 -1，并设置全局变量 errno 表示具体错误。
    */
    /*
    在 C++ 的 Socket 编程中，socket() 函数返回的 sockfd（Socket File Descriptor）是一个 文件描述符（File Descriptor），
    它是操作系统用来标识和管理 Socket 资源的唯一整数。
    */
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 填写client需要连接的server信息ip+port
    /*
    sockaddr_in 是 C/C++ 网络编程中用于表示 IPv4 地址和端口号的结构体，属于 <netinet/in.h> 头文件。
    其核心作用是为套接字操作（如绑定、连接等）提供格式化的地址信息，确保数据在网络中正确传输。
    */
    sockaddr_in server;
    /*
    C++ 中的 memset 函数用于将一块内存区域的每个字节设置为指定的值。
    将 ptr 指向的内存块的前 num 个字节逐个字节设置为 value。
    */
    memset(&server, 0, sizeof(sockaddr_in)); // 清空结构体

    /*
    sin_family 是 sockaddr_in 结构体中的一个字段，用于指定地址族（Address Family）。
    AF_INET 表示使用IPv4地址族。IPv4是互联网协议版本4，是目前广泛使用的IP地址格式。
    */
    server.sin_family = AF_INET; // IPv4 地址族
    /*
    sin_port 是 sockaddr_in 结构体中的一个字段，用于指定端口号。
    htons() 是一个函数，用于将主机字节序（通常是Little-Endian）转换为网络字节序（Big-Endian）。
                         网络字节序是网络通信中使用的标准字节序。
    8080 是端口号，通常用于HTTP代理服务器或其他服务
    */
    server.sin_port = htons(port); // 端口号转换为网络字节序
    /*ss
    sin_addr 是 sockaddr_in 结构体中的一个字段，用于存储IP地址。
    s_addr 是 sin_addr 结构体中的一个字段，用于存储IP地址的二进制形式。
    inet_addr() 是一个函数，用于将点分十进制的IP地址字符串（如 "192.168.1.1"）转换为32位的二进制格式（网络字节序）。
    */
    server.sin_addr.s_addr = inet_addr(ip); // IP 转为二进制格式

    // client和server进行连接
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // main线程用于接受用户输入，负责发送数据
    for (;;)
    {
        // 显示首页面菜单 的登陆、注册、退出
        cout << "====================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "====================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice)
        {
        case 1: // login业务
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); //
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0); // 长度加一是\0，不然接收方可能无法确定结束边界
            if (len == -1)
            {
                cerr << "send login msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (-1 == len)
                {
                    cerr << "recv login response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if (0 != responsejs["errno"].get<int>()) // 登录失败
                    {
                        cerr << responsejs["errmsg"] << endl;
                    }
                    else // 登录成功
                    {
                        // 记录当前用户的id和name
                        g_currentUser.setId(responsejs["id"].get<int>());
                        g_currentUser.setName(responsejs["name"]);

                        // 记录当前用户的好友列表信息
                        if (responsejs.contains("friends"))
                        {
                            // 初始化
                            g_currentUserFriendList.clear();

                            vector<string> vec = responsejs["friends"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                        }

                        // 记录当前用户群组列表信息
                        if (responsejs.contains("groups"))
                        {
                            // 初始化
                            g_currentUserGroupList.clear();

                            vector<string> vec1 = responsejs["groups"];
                            for (string &groupstr : vec1)
                            {
                                json grpjs = json::parse(groupstr);
                                Group group;
                                group.setId(grpjs["id"].get<int>());
                                group.setName(grpjs["groupname"]);
                                group.setDesc(grpjs["groupdesc"]);

                                vector<string> vec2 = grpjs["users"];
                                for (string &userstr : vec2)
                                {
                                    GroupUser user;
                                    json js = json::parse(userstr);
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    user.setRole(js["role"]);
                                    group.getUsers().push_back(user);
                                }

                                g_currentUserGroupList.push_back(group);
                            }
                        }

                        // 显示用户的基本信息
                        showCurrentUserData();

                        // 显示当前用户的离线消息 个人聊天信息或者群组消息
                        if (responsejs.contains("offlinemsg"))
                        {
                            vector<string> vec = responsejs["offlinemsg"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                // time + [id] + name + " said: " +xxx
                                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
                                    continue;
                                }
                                else
                                {
                                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
                                    continue;
                                }
                            }
                        }

                        // 登录成功，启动接受线程负责接收数据，该线程只启动一次
                        static int readthreadnumber = 0;
                        if (readthreadnumber == 0)
                        {
                            std::thread readTask(readTaskHandler, clientfd); // 底层linux调用pthread_create
                            /*
                            std::thread 的 detach 函数用于将线程与其关联的 std::thread 对象分离。
                            分离后，线程将在后台独立运行，std::thread 对象不再管理该线程的生命周期。
                            */
                            readTask.detach(); // 底层linux调用pthread_detach
                            readthreadnumber++;
                        }

                        // 进入聊天主页面
                        isMainMenuRunning = true;
                        mainMenu(clientfd);
                    }
                }
            }
        }
        break;
        case 2: // register业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50); // cin>> scanf 不能带空格
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (-1 == len)
                {
                    cerr << "recv reg response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if (0 != responsejs["errno"].get<int>()) // 注册失败
                    {
                        cerr << name << " is already exist, register error!" << endl;
                    }
                    else // 注册成功
                    {
                        cout << name << " register success, userid is " << responsejs["id"]
                             << ", do  not forget it!" << endl;
                    }
                }
            }
        }
        break;
        case 3: // quit业务
            close(clientfd);
            exit(0);
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }

    return 0;
}

// 接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (-1 == len || 0 == len)
        {
            close(clientfd);
        }

        // 接受ChatServer转发的数据，反序列化生成json对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
            continue;
        }
        else if (GROUP_CHAT_MSG == msgtype)
        {
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
            continue;
        }
    }
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    cout << "=================login user=====================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "-----------------friend list--------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "-----------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "===========================================" << endl;
}

//"help" command handler
void help(int fd = 0, string str = "");
//"chat" command handler
void chat(int, string);
//"addfriend" command handler
void addfriend(int, string);
//"creategroup" command handler
void creategroup(int, string);
//"addgroup" command handler
void addgroup(int, string);
//"groupchat" command handler
void groupchat(int, string);
//"quit" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}

};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>>
    commandHandlerMap = {
        {"help", help},
        {"chat", chat},
        {"addfriend", addfriend},
        {"creategroup", creategroup},
        {"addgroup", addgroup},
        {"groupchat", groupchat},
        {"loginout", loginout}};

// 主聊天页面程序
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}

//"help" command handler
void help(int, string)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

//"addfriend" command handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}

//"chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

//"creategroup" command handler
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup commmand invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx - 1);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup error -> " << buffer << endl;
    }
}

//"addgroup" command handler
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup error -> " << buffer << endl;
    }
}

//"groupchat" command handler groupchat:groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command is invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx - 1);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}

//"quit" command handler
void loginout(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    time_t now = time(nullptr);
    tm local_time;
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime_r(&now, &local_time));
    return buffer;
}