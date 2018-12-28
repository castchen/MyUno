#include <iostream>
#include <string>

#include "mysql_db.h"

using namespace std;

int main()
{
    MysqlDb sql;
    int ret = sql.Init("127.0.0.1", "root", "123456", "uno");
    if (ret)
    {
        cout << "init error" << endl;
    }

    DbAccountInfo acc;
    ret = sql.GetAccountInfo("test", acc);
    if (ret)
    {
        cout << "get error " << ret << endl;
    }
    else
    {
        cout << "uid: " << acc.uid_ << endl;
        cout << "acc: " << acc.account_ << endl;
        cout << "pas: " << acc.passwd_ << endl;
    }

    return 0;
}