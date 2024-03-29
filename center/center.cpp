﻿#include "center.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <regex>

using namespace std;
using namespace ol;

#define CENTER_CONF "ip"

void Center::Main()
{
    /// 只审计运行之后的事件
    /// 找到最后一个事件，取到id号
    int last_id = 0;
    auto rows = db_->GetResult("select max(id) from t_log");
    if (rows[0][0].data)
    {
        last_id = atoi(rows[0][0].data);
    }
    cout << "last id is: " << last_id << endl;

    /// 获取审计策略
    rows = db_->GetResult("select * from t_strategy");
    /// 正则表达式map，key 审计事件名称
    map<string, regex> strategys;
    for (const auto& row : rows)
    {
        if (row[1].data && row[2].data)
            strategys[row[1].data] = regex(row[2].data);
    }

    for (;;)
    {
        /// 获取Agent存储的最新数据
        char buf[1024] = { 0 };
        sprintf(buf, "select * from t_log where id>%d", last_id);
        auto rows = db_->GetResult(buf);
        if (rows.empty())
        {
            this_thread::sleep_for(100ms);
        }       

        for (const auto row : rows)
        {
            last_id = atoi(row[0].data);
            if (!row[2].data)
                continue;
            cout << row[2].data << endl;

            for (const auto& strategy : strategys)
            {
                /// 正则结果
                smatch match;
                string log = row[2].data;
                /// 匹配正则，返回结果到match
                bool ret = regex_match(log, match, strategy.second);
                if (!ret || match.size() <= 0) continue;

                cout << strategy.first << endl;

                KVData data;
                /// 审计成功的名称
                data["name"] = strategy.first.c_str();
                data["context"] = log.c_str();
                if (row[1].data)
                    data["device_ip"] = row[1].data;
                /// 匹配结果：下标 0 是整个字符串 1 是第一个匹配结果
                string user = match[2];
                string from_ip = match[3];
                string port = match[4];
                data["user"] = user.c_str();
                data["from_ip"] = from_ip.c_str();
                data["port"] = port.c_str();
                db_->Insert(data, "t_audit");
            }
        }
    }
}

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
    
    return CreateTable();
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

bool Center::AddDevice(std::string ip, std::string name)
{
    KVData data;
    data["ip"] = ip.c_str();
    data["name"] = name.c_str();
    return db_->Insert(data, "t_device");
}

bool Center::CreateTable()
{
    /// 创建策略表
    /// 清理原来数据，防止数据污染
    db_->StartTransaction();
    bool ret = db_->Query("drop table if exists `t_strategy`");
    if (!ret)
    {
        db_->Rollback();
        return false;
    }
    string sql = "CREATE TABLE IF NOT EXISTS `t_strategy`(                      \
                 `id` INT AUTO_INCREMENT,                                       \
                 `name` VARCHAR(256) CHARACTER SET 'utf8' COLLATE 'utf8_bin',   \
                 `strategy` VARCHAR(4096),                                      \
                  PRIMARY KEY(`id`))";
    ret = db_->Query(sql.c_str(), sql.size());
    if (!ret)
    {
        db_->Rollback();
        return false;
    }

    KVData data;
    data["name"] = "用户登陆失败";
    data["strategy"] = ".*Failed (.+) for (.+) from ([0-9.]+) port ([0-9]+).*";
    ret = db_->Insert(data, "t_strategy");
    if (!ret)
    {
        cout << "insert strategy failed!" << endl;
        db_->Rollback();
        return false;
    }

    data["name"] = "用户登陆成功";
    data["strategy"] = ".*Accepted (.+) for (.+) from ([0-9.]+) port ([0-9]+).*";
    ret = db_->Insert(data, "t_strategy");
    if (!ret)
    {
        cout << "insert strategy failed!" << endl;
        db_->Rollback();
        return false;
    }

    /// 创建用户表, 初始化管理员用户 root 123456 md5
    ret = db_->Query("drop table if exists `t_user`");
    if (!ret)
    {
        db_->Rollback();
        return false;
    }
    sql = "CREATE TABLE IF NOT EXISTS `t_user`(                                 \
                 `id` INT AUTO_INCREMENT,                                       \
                 `user` VARCHAR(256) CHARACTER SET 'utf8' COLLATE 'utf8_bin',   \
                 `pass` VARCHAR(1024),                                          \
                  PRIMARY KEY(`id`))";
    ret = db_->Query(sql.c_str(), sql.size());
    if (!ret)
    {
        db_->Rollback();
        return false;
    }

    KVData add_user;
    add_user["user"] = "root";
    add_user["@pass"] = "md5('123456')";
    ret = db_->Insert(add_user, "t_user");
    if (!ret)
    {
        cout << "insert user failed!" << endl;
        db_->Rollback();
        return false;
    }

    /// 创建日志表
    ret = db_->Query("drop table if exists `t_log`");
    if (!ret)
    {
        db_->Rollback();
        return false;
    }
    sql = "CREATE TABLE IF NOT EXISTS `t_log`(           \
                  `id`  INT AUTO_INCREMENT,              \
                  `ip`  VARCHAR(16),                     \
                  `log` VARCHAR(2048),                   \
                  `log_time` DATETIME,                   \
                  PRIMARY KEY(`id`))";
    ret = db_->Query(sql.c_str(), sql.size());
    if (!ret)
    {
        db_->Rollback();
        return false;
    }

    /// 创建设备表 t_device
    ret = db_->Query("drop table if exists `t_device`");
    if (!ret)
    {
        db_->Rollback();
        return false;
    }
    sql = "CREATE TABLE IF NOT EXISTS `t_device`(         \
                  `id`  INT AUTO_INCREMENT,               \
                  `ip`  VARCHAR(16),                      \
                  `name` VARCHAR(2048),                   \
                  `last_heart` DATETIME,                  \
                  PRIMARY KEY(`id`))";
    ret = db_->Query(sql.c_str(), sql.size());
    if (!ret)
    {
        db_->Rollback();
        return false;
    }

    /// 创建审计结果表 t_audit
    ret = db_->Query("drop table if exists `t_audit`");
    if (!ret)
    {
        db_->Rollback();
        return false;
    }
    sql = "CREATE TABLE IF NOT EXISTS `t_audit`(          \
                  `id`  INT AUTO_INCREMENT,               \
                  `name` VARCHAR(256),                    \
                  `context` VARCHAR(2048),                \
                  `user` VARCHAR(256),                    \
                  `device_ip`  VARCHAR(16),               \
                  `from_ip`  VARCHAR(16),                 \
                  `port` INT,                             \
                  `last_heart` DATETIME,                  \
                  PRIMARY KEY(`id`))";
    ret = db_->Query(sql.c_str(), sql.size());
    if (!ret)
    {
        db_->Rollback();
        return false;
    }

    return db_->Commit();
}

Center::Center()
{
}

Center::~Center()
{
}