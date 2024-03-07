#include "agent.h"

#include <iostream>
#include <thread>
#include <cstring>

#ifndef _WIN32
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif // !_WIN32


using namespace ol;
using namespace std;

#define LOGPATH "/var/log/auth.log"

std::string Agent::GetLocalIP()
{
    char ip[16] = { 0 };
#ifndef _WIN32
    ifaddrs* ifaddr = NULL;
    if (getifaddrs(&ifaddr) != 0) return "";

    /// 遍历地址
    ifaddrs* iter = ifaddr;
    while (iter != NULL)
    {
        /// ipv4
        if (iter->ifa_addr->sa_family == AF_INET)
        {
            if (strcmp(iter->ifa_name, "lo") != 0)  // 去掉回环地址，127.0.0.1
            {
                void* temp = &((sockaddr_in*)iter->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, temp, ip, INET_ADDRSTRLEN);
                break;
            }
        }
        iter = iter->ifa_next;
    }

    freeifaddrs(ifaddr);
#endif // !_WIN32
    return ip;
}

bool Agent::CreateTable()
{
    /// 创建t_log日志表
    string sql = "CREATE TABLE IF NOT EXISTS `t_log`(    \
                  `id`  INT AUTO_INCREMENT,              \
                  `ip`  VARCHAR(16),                     \
                  `log` VARCHAR(2048),                   \
                  `log_time` DATETIME,                   \
                  PRIMARY KEY(`id`))";
    return db_->Query(sql.c_str(), sql.size());
}

bool Agent::Init(string ip)
{
    local_ip_ = GetLocalIP();
    cout << local_ip_ << endl;
    if (ip.empty())
    {
        cerr << "Agent::Init failed! ip is empty" << endl;
        return false;
    }

    if (db_ == nullptr)
        db_ = new OrionLinkDB;
    
    if (!db_->Connect(ip.c_str(), "root", "123456", "ATM"))
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
            SaveLog(log);
            log = "";
            continue;
        }
        log += c;
    }
}

bool Agent::SaveLog(std::string log)
{
    cout << log << endl;
    ol::KVData data;
    data["log"] = log.c_str();
    data["ip"] = local_ip_.c_str();
    data["@log_time"] = "now()";

    return db_->Insert(data, "t_log");;
}

Agent::Agent()
{
}

Agent::~Agent()
{
}

