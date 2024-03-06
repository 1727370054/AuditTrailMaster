#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <string>

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

    std::string InputPassword();

    /// @brief 最大登陆失败次数
    int max_login_times = 10;
private:
    Client();
    Client(const Client&) =delete;
    Client& operator=(const Client&) = delete;

private:
    ol::OrionLinkDB* db_ = NULL;
};

#endif // __CLIENT_H__


