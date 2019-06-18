#ifndef __GAME_H__
#define __GAME_H__

#include <map>
#include <memory>

#include "player.h"
#include "table.h"
#include "mysql_db.h"
#include "proto/uno.pb.h"
#include "cmd_list.h"

class Game
{
public:
    RetCodeList error_no() const { return error_no_; }

    void set_error_no(RetCodeList val) { error_no_ = val; }

    int ep_fd() const { return ep_fd_; }

    void set_ep_fd(int val) { ep_fd_ = val; }

public:
    int PlayerConnect(int fd, std::shared_ptr<Player> player);

    int PlayerLogOut(std::shared_ptr<Player> player);

    int PlayerLogIn(const std::string &message, std::shared_ptr<Player> player);

    std::shared_ptr<Player> GetPlayByFd(int fd);

    int InitDb(const std::string &host, const std::string &user, const std::string &passwd, const std::string &db_name);

    int ExcuteCmd(int fd, int cmd, const std::string &message);

    int GetAccountInfo(std::string account, DbAccountInfo &info);

    int GetInfoInfo(int uid, DbInfoInfo &info);

private:
    std::map<int, std::shared_ptr<Table>> table_list_;
    std::map<int, std::shared_ptr<Player>> fd_players_;
    std::map<int, std::shared_ptr<Player>> uid_players_;
    MysqlDb mysql_;
    RetCodeList error_no_ = RetCodeList::kCorrect;
    int ep_fd_ = 0;
};
#endif