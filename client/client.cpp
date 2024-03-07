#include "client.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <chrono>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
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
using namespace chrono;

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

void Client::c_log(std::vector<std::string> cmds)
{
    int page = 1;
    int pagecount = 10;
    if (cmds.size() > 1)
        page = atoi(cmds[1].c_str());
    if (cmds.size() > 2)
        pagecount = atoi(cmds[2].c_str());

    stringstream ss;
    ss << "select * from t_log limit ";
    ss << (page - 1) * pagecount << ", ";
    ss << pagecount;

    int total = 0;
    auto rows = db_->GetResult(ss.str().c_str());
    cout << endl;
    SplitLine("t_log");
    for (const auto& row : rows)
    {
        total++;
        for (const auto data : row)
        {
            if (data.data)
                cout << data.data << " ";
        }
        cout << endl;
    }

    cout << "Total: " << total << endl;
    cout << "Page: " << page << " | " << "PageCount: " << pagecount << endl;
    SplitLine("t_log");
}

void Client::c_audit(std::vector<std::string> cmds)
{
    int total = 0;
    string sql = "select * from t_audit";
    auto rows = db_->GetResult(sql.c_str());
    //遍历每一行
    cout << endl;
    SplitLine("t_audit");
    for (auto row : rows)
    {
        total++;
        //遍历每一列
        for (auto c : row)
        {
            if (c.data) cout << c.data << " ";
        }
        cout << endl;
    }
    cout << "Total: " << total << endl;
    SplitLine("t_audit");
}

void Client::c_test(std::vector<std::string> cmds)
{
    int count = 100000;
    if (cmds.size() > 1)
        count = atoi(cmds[1].c_str());
    db_->StartTransaction();
    for (int i = 0; i < count; i++)
    {
        KVData data;
        stringstream ss;
        ss << "testlog";
        ss << (i + 1);
        string tmp = ss.str();
        data["log"] = tmp.c_str();
        data["ip"] = "127.0.0.1";
        data["@log_time"] = "now()";
        db_->Insert(data, "t_log");

    }

    {
        KVData data;
        stringstream ss;
        ss << "search001";
        string tmp = ss.str();
        data["log"] = tmp.c_str();
        data["ip"] = "10.0.0.1";

        //插入时间，用mysql now（）
        //@表示 字段内容不加引号，@会自动去除
        data["@log_time"] = "now()";
        db_->Insert(data, "t_log");

    }
    db_->Commit();
}

void Client::c_search(std::vector<std::string> cmds)
{
    if (cmds.size() < 2) return;
    string key = cmds[1];
    
    /// 记录开始时间
    auto start = system_clock::now();
    string sql = "select * from t_log ";
    string where = " where ip='";
    where += key;
    where += "'";
    sql += where;
    //一百万数据 无索引 0.47秒 有索引 0.000687
    SplitLine("search");
    auto rows = db_->GetResult(sql.c_str());
    for (auto row : rows)
    {
        //遍历每一列
        for (auto c : row)
        {
            if (c.data) cout << c.data << " ";
        }
        cout << endl;
    }

    /// 记录结束时间
    auto end = system_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "times sec: " << double(duration.count()) * microseconds::period::num / microseconds::period::den << " s" << endl;

    //统计总数
    sql = "select count(*) from t_log ";
    sql += where;
    rows = db_->GetResult(sql.c_str());
    int total = 0;
    if (rows.size() > 0 && rows[0][0].data)
        total = atoi(rows[0][0].data);
    cout << "total :" << total << endl;
    SplitLine("search");
}

void Client::c_like(std::vector<std::string> cmds)
{
    if (cmds.size() < 2) return;
    string key = cmds[1];

    /// 记录开始时间
    auto start = system_clock::now();
    string sql = "select * from t_log ";
    string where = " where log like'%";
    where += key;
    where += "%'";
    sql += where;

    SplitLine("like");
    auto rows = db_->GetResult(sql.c_str());
    for (auto row : rows)
    {
        //遍历每一列
        for (auto c : row)
        {
            if (c.data) cout << c.data << " ";
        }
        cout << endl;
    }

    /// 记录结束时间
    auto end = system_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "times sec: " << double(duration.count()) * microseconds::period::num / microseconds::period::den << " s" << endl;

    //统计总数
    sql = "select count(*) from t_log ";
    sql += where;
    rows = db_->GetResult(sql.c_str());
    int total = 0;
    if (rows.size() > 0 && rows[0][0].data)
        total = atoi(rows[0][0].data);
    cout << "total :" << total << endl;
    SplitLine("like");
}

void Client::SplitLine(std::string content)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    // 获取控制台窗口的句柄
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        std::cerr << "获取控制台信息失败。" << std::endl;
        return;
    }

    // 计算窗口宽度
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    // 生成等号("=")字符串
    int size = content.size();
    std::string separator((columns / 2) - size, '-');

    // 输出
    std::cout << separator << " ";
    std::cout << content << " ";
    std::cout << separator << std::endl;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    // 获取终端的宽度
    int width = w.ws_col;

    // 根据终端宽度生成等号("=")字符串
    int size = content.size();
    std::string separator((width / 2) - size, '-');

    // 输出
    std::cout << separator << " ";
    std::cout << content << " "; // 你可以在这里输出你想要的内容
    std::cout << separator << std::endl;
#endif // !_WIN32
}

void Client::Main()
{
    /// 用户登陆
    if (!Login()) return;

    /// 分页显示 t_log
    // 获取用户输入
    for (;;)
    {
        cout << "client >>" << flush;
        string cmd = Input();
        
        // log 1 10 第一页 一页10行
        vector<string> cmds;
        char* s = strtok((char*)cmd.c_str(), " ");
        while (s)
        {
            cmds.push_back(s);
            s = strtok(0, " ");
        }
        string type = cmd;
        if (cmds.size() > 0)
        {
            type = cmds[0];
        }

        if (type == "log")
        {
            c_log(cmds);
        }
        else if (type == "audit")
        {
            c_audit(cmds);
        }
        else if (type == "test")
        {
            c_test(cmds);
        }
        else if (type == "search")
        {
            c_search(cmds);
        }
        else if (type == "like")
        {
            c_like(cmds);
        }
        else if (type == "quit")
        {
            exit(0);
        }
    }
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

std::string Client::Input()
{
    string input = "";
    for (;;)
    {
        char c = getchar();
        if (c <= 0 || c == '\n' || c == '\r')
            break;
        input += c;
    }
    return input;
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
