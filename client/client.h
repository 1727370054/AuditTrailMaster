﻿#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <string>
#include <vector>

#include "../orion_link_db/orion_link_db.h"

class Client
{
public:
    ~Client();

    static Client* GetInstance()
    {
        static Client c;
        return &c;
    }

    bool Init(std::string ip);

    /// @brief 主循环
    void Main();

    /// @brief 登陆
    bool Login();

    /// @brief 接收密码输入
    /// @return 返回输入的密码
    std::string InputPassword();

    std::string Input();

    /// @brief 检查用户输入，防止注入攻击
    /// @return true 安全，false有违规 
    bool CheckInput(const std::string& in);

    /// @brief 最大登陆失败次数
    int max_login_times = 10;
private:
    void c_log(std::vector<std::string> cmds);
    /// @brief 审计结果显示 audit 
    void c_audit(std::vector<std::string>cmds);
    /// @brief test 10000 插入一万条测试数据
    void c_test(std::vector<std::string>cmds);
    /// @brief search 10.0.0.1 搜索ip
    void c_search(std::vector<std::string>cmds);
    /// @brief 模糊匹配
    void c_like(std::vector<std::string>cmds);

    void SplitLine(std::string content);

    Client();
    Client(const Client&) =delete;
    Client& operator=(const Client&) = delete;

private:
    ol::OrionLinkDB* db_ = NULL;
};

#endif // __CLIENT_H__


