#include "center.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace ol;

#define CENTER_CONF "ip"

bool Center::Install(std::string ip)
{
    /// 生成配置文件 数据库的IP
    ofstream ofs;
    ofs.open(CENTER_CONF);
    if (!ofs.is_open())
    {
        cout << "open conf " << CENTER_CONF << " failed!" << endl;
        return false;
    }

    ofs << ip;
    ofs.close();

    /// 初始化表和数据
    if (!Init())
        return false;

    return true;
}

bool Center::Init()
{
    if (db_ == NULL)
    {
        db_ = new OrionLinkDB();
    }

    ifstream ifs;
    ifs.open(CENTER_CONF);
    if (!ifs.is_open())
    {
        cout << "open conf " << CENTER_CONF << " failed!" << endl;
        cout << "please install center" << endl;
        return false;
    }

    string ip = "";
    ifs >> ip;
    ifs.close();

    if (ip.empty())
    {
        cout << "ip is empty, please install center" << endl;
        return false;
    }
    
    cout << "init center " << ip << endl;
    if (!db_->Connect(ip.c_str(), "root", "123456", "ATM"))
    {
        cout << "database connect failed!" << endl;
        return false;
    }

    cout << "database connect success!" << endl;

    return db_->Query("set names utf8");
}

Center::Center()
{
}

Center::~Center()
{
}