#ifndef __GAME_H__
#define __GAME_H__

#include <map>
#include <memory>

#include "player.h"
#include "table.h"
#include "mysql_db.h"
#include "proto/uno.pb.h"

class Game
{
public:
    uno::Error_ErrorCode error_no() const { return error_no_; }

    void set_error_no(uno::Error_ErrorCode val) { error_no_ = val; }

    int ep_fd() const { return ep_fd_; }

    void set_ep_fd(int val) { ep_fd_ = val; }

public:
    int PlayerConnect(int fd, std::shared_ptr<Player> player);

    int PlayerLogOut(std::shared_ptr<Player> player);

    int PlayerLogIn(const std::string &message, std::shared_ptr<Player> player);

    std::shared_ptr<Player> GetPlayByFd(int fd);

    int InitDb();

    int ExcuteCmd(int fd, int cmd, const std::string &message);

    int GetAccountInfo(std::string account, DbAccountInfo &info);

    int GetInfoInfo(int uid, DbInfoInfo &info);

private:
    std::map<int, std::shared_ptr<Table>> table_list_;
    std::map<int, std::shared_ptr<Player>> fd_players_;
    std::map<int, std::shared_ptr<Player>> uid_players_;
    MysqlDb mysql_;
    uno::Error_ErrorCode error_no_ = uno::Error_ErrorCode_CORRECT;
    int ep_fd_ = 0;
};
#endif