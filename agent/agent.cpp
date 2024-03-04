#include "agent.h"

#include <iostream>
#include <thread>

using namespace ol;
using namespace std;

#define LOGPATH "/var/log/auth.log"

bool Agent::Init(string ip)
{
    if (ip.empty())
    {
        cerr << "Agent::Init failed! ip is empty" << endl;
        return false;
    }

    if (db_ == nullptr)
        db_ = new OrionLinkDB;
    
    if (!db_->Connect(ip.c_str(), "root", "123456", "agent"))
    {
        cerr << "Agent::Init failed! connect SQL error" << endl;
        return false;
    }

    cout << "mysql connect success!" << endl;

    /// 读取日志文件
    ifs.open(LOGPATH, ios::in | ios::binary);

    if (!ifs.is_open())
    {
        cerr << "Agent::Init failed! open " << LOGPATH << " error" << endl;
        return false;
    }

    cerr << "open " << LOGPATH << " success!" << endl;

    /// 只审计系统开始运行之后的事件
    /// 移动到结尾
    ifs.seekg(0, ios::end);

    return true;
}

void Agent::Main()
{
    string log = "";

    char c;
    while (true) 
    {
        if (!ifs.get(c)) 
        {
            ifs.clear(); // 清除错误标志
            ifs.seekg(0, std::ios::end); // 移动到文件末尾准备读取新内容
            std::this_thread::sleep_for(100ms);
            continue;
        }
        if (c == '\n')
        {
            cout << log << endl;
            log = "";
            continue;
        }
        log += c;
    }
}

Agent::Agent()
{
}

Agent::~Agent()
{
}

