#include "client.h"

#include <iostream>
#ifdef _WIN32
#include <conio.h>
#else
#include <termio.h>
char _getch()
{
    termios new_tm;
    /// 原来的模式
    termios old_tm;
    int fd = 0;
    if (tcgetattr(fd, &old_tm) < 0)
        return -1;
    /// 更改为原始模式，没有回显
    cfmakeraw(&new_tm);
    if (tcsetattr(fd, TCSANOW, &new_tm) < 0)
    {
        return -1;
    }
    char c = getchar();
    if (tcsetattr(fd, TCSANOW, &old_tm) < 0)
    {
        return -1;
    }

    return c;
}
#endif // _WIN32

using namespace std;
using namespace ol;

bool Client::Init(std::string ip)
{
    if (db_ == NULL)
        db_ = new OrionLinkDB();
    if (!db_->Connect(ip.c_str(), "root", "123456", "ATM"))
    {
        cerr << "database connect failed!" << endl;
        return false;
    }
    cerr << "database connect success!" << endl;
    return db_->Query("set names utf8");
}

void Client::Main()
{
    /// 用户登陆
    Login();
}

bool Client::Login()
{
    bool is_login = false;

    for (int i = 0; i < max_login_times; i++)
    {
        string username = "";
        cout << "input username: " << flush;
        cin >> username;
        cout << "[" << username << "]" << endl;

        string password = "";
        cout << "input password: " << flush;
        password = InputPassword();
        cout << "[" << password << "]" << endl;
    }

    return is_login;
}

std::string Client::InputPassword()
{
    /// 清空缓冲
    cin.ignore(4096, '\n');
    string password = "";
    for (;;)
    {
        char a = _getch(); /// 获取输入字符不显示
        if (a <= 0 || a == '\n' || a == '\r') break;
        password += a;
        cout << "*" << flush;
    }

    return password;
}

Client::Client()
{
}

Client::~Client()
{
}
