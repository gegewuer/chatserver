#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

//匹配User表的ORM类
//ORM（对象关系映射，Object-Relational Mapping）类是一种编程技术，用于在面向对象编程语言中实现对象与关系数据库之间的映射。
//通过ORM类，开发者可以使用面向对象的方式操作数据库，而不必直接编写SQL语句。
class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getPwd() { return this->password; }
    string getState() { return this->state; }

private:
    int id;
    string name;
    string password;
    string state;
};

#endif