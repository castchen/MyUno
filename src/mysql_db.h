#ifndef __MYSQL_DB_H__
#define __MYSQL_DB_H__

#include <mysql/mysql.h>
#include <string>

struct DbAccountInfo
{
    int uid_ = 0;
    std::string account_;
    std::string passwd_;
};

struct DbInfoInfo
{
    int uid_ = 0;
    std::string name_;
    int score_ = 0;
};

class MysqlDb
{
public:
    MysqlDb() = default;

    ~MysqlDb();

    int Init(std::string host, std::string user, std::string passwd, std::string db_name);

    int GetAccountInfo(std::string account, DbAccountInfo &info);

    int GetInfoInfo(int uid, DbInfoInfo &info);

private:
    MYSQL *mysql_ = NULL;
};

#endif