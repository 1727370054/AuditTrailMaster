#include "client.h"

#include <iostream>
#include <sstream>
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

        /// 注入攻击
        /// select id from t_user where user='root' and pass=md5('123456');
        /// select id from t_user where user='1'or'1'='1' and pass=md5('1')or'c4ca4238a0b923820dcc509a6f75849b'=md5('1');
        /// username = 1'or'1'='1
        /// password = 1')or'c4ca4238a0b923820dcc509a6f75849b'=md5('1
        string password = "";
        cout << "input password: " << flush;
        password = InputPassword();
        cout << endl;
        if (!CheckInput(password) || !CheckInput(username))
        {
            cout << "Injection attacks!" << endl;
            continue;
        }

        stringstream sql;
        sql << "select id from t_user where user='" << username;
        sql << "' and pass=md5('" << password << "')";

        auto rows = db_->GetResult(sql.str().c_str());
        if (rows.size() > 0)
        {
            cout << "login success!" << endl;
            is_login = true;
            break;
        }

        cout << "login failed!" << endl;
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

bool Client::CheckInput(const std::string& in)
{
    /// 限定不允许出现的字符
    string str = "'\"()";
    for (char c : str)
    {
        size_t found = in.find(c);
        if (found != string::npos)  /// 发现违规字符
        {
            return false;
        }
    }

    return true;
}

Client::Client()
{
}

Client::~Client()
{
}
