#include"json.hpp"
using json = nlohmann::json;

#include<iostream>
#include<vector>
#include<map>
#include<string>
using namespace std;

//json序列化实例
void func1(){
    json js;
    js["msg_type"] = 2;
    js["from"]="zhang san";
    js["to"]="li si";
    js["msg"]="hello,what are you doing now?";

    string sendBuf = js.dump();
    //cout<<endl;
    //cout<<sendBuf<<endl;
    cout<<sendBuf.c_str()<<endl;
    //cout<<"second"<<endl;

}

void func2(){
    json js;
    
    js["msg_type"] = 2;
    js["from"]="zhang san";
    js["to"]="li si";
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["li si"] = "hello china";
    js["id"]={1,2,3,4,5};

    string jsonBuf=js.dump();//序列化
    cout<<jsonBuf.c_str()<<endl;//模仿字节流/字符流发送


}

string func3(){
    json js;
    js["boluo"] = "chuixue";
    js["lu"] = "xiaoguo";

    return js.dump();
}

string func4(){
    json js;
    js["boluo"] = "chuixue";
    js["lu"] = "xiaoguo";

    return js.dump().c_str();
}

string func5(){
    nlohmann::json js;
    vector<int> vec{1,3,5,7};
    vec.push_back(2);
    js["vector"]=vec;
    return js.dump();
    

}

int main(){
    json jsBuf;
    
    //jsBuf = json::parse(func4());//反序列化
    //cout<<jsBuf<<endl;

    jsBuf=json::parse(func5());
    cout<<jsBuf<<endl;
    vector<int> vec=jsBuf["vector"];
    for(auto it:vec){
        cout<<it<<" ";
    }
    cout<<endl;

    return 0;
}