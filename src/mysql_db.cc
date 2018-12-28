#include <string>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <sstream>

#include "mysql_db.h"

MysqlDb::~MysqlDb()
{
    if (mysql_ != NULL)
    {
        mysql_close(mysql_);
    }
}

int MysqlDb::Init(std::string host, std::string user, std::string passwd, std::string db_name)
{
    mysql_ = mysql_init(NULL);
    if (mysql_ == NULL)
    {
        return -1;
    }

    mysql_real_connect(mysql_, host.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), 0, NULL, 0);
    if (mysql_ == NULL)
    {
        return -2;
    }

    return 0;
}

int MysqlDb::GetAccountInfo(std::string account, DbAccountInfo &info)
{
    std::string sql = "select * from account where account = \"" + account + "\"";
    if (mysql_query(mysql_, sql.c_str()))
    {
        return -1;
    }
    else
    {
        MYSQL_RES *result = mysql_store_result(mysql_);
        if (result)
        {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row <= 0)
            {
                return -2;
            }
            info.uid_ = atoi(row[0]);
            info.account_ = row[1];
            info.passwd_ = row[2];

            return 0;
        }
        else
        {
            return -3;
        }

        mysql_free_result(result);
    }
}

int MysqlDb::GetInfoInfo(int uid, DbInfoInfo &info)
{
    std::ostringstream oss;
    oss << "select * from info where uid = " << uid;
    if (mysql_query(mysql_, oss.str().c_str()))
    {
        return -1;
    }
    else
    {
        MYSQL_RES *result = mysql_store_result(mysql_);
        if (result)
        {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row <= 0)
            {
                return -2;
            }
            info.uid_ = atoi(row[0]);
            info.name_ = row[1];
            info.score_ = atoi(row[2]);

            return 0;
        }
        else
        {
            return -3;
        }

        mysql_free_result(result);
    }
}
