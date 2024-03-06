#ifndef __AGENT_H__
#define __AGENT_H__

#include "../orion_link_db/orion_link_db.h"

#include <string>
#include <fstream>

class Agent
{
public:
    ~Agent();

    static Agent* GetInstance()
    {
        static Agent a;
        return &a;
    }

    /// @brief agent模块初始化，只能调用一次，在所有接口之前调用
    bool Init(std::string ip);

    /// @brief 主循环
    void Main();

    /// @brief 日志写入数据库
    bool SaveLog(std::string log);

    static std::string GetLocalIP();
private:
    bool CreateTable();

    Agent();
    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;
private:
    /// 数据库对象
    ol::OrionLinkDB* db_ = nullptr;
    /// 读取日志文件
    //FILE* fp_ = nullptr;
    std::ifstream ifs;

    std::string local_ip_ = "";
};

#endif // __AGENT_H__
