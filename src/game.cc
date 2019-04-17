#include "game.h"

extern Log kLog;

int Game::PlayerConnect(int fd, std::shared_ptr<Player> player)
{
    if (fd_players_.find(fd) != fd_players_.end())
    {
        kLog.Error("fd_players_ has fd:%d\n", fd);
        return -1;
    }

    fd_players_[fd] = player;
    return 0;
}

int Game::PlayerLogOut(std::shared_ptr<Player> player)
{
    if (player == nullptr)
    {
        kLog.Error("player null\n");
        return 0;
    }

    std::vector<int> fd_vec;
    player->LogOut(fd_vec);
    for (auto fd : fd_vec)
    {
        auto iter = fd_players_.find(fd);
        if (iter != fd_players_.end())
        {
            fd_players_.erase(iter);
        }
        kLog.Info("uid:%d close fd:%d\n", player->uid(), fd);
    }

    //在没开始游戏之前是可以从uid_players里面删掉的
    if (player->status() < Player::kInTheGame)
    {
        auto iter = uid_players_.find(player->uid());
        if (iter != uid_players_.end())
        {
            uid_players_.erase(iter);
            kLog.Info("uid:%d erase in uid_players.\n", player->uid());
        }
    }

    kLog.Info("Player Disconnect. uid:%d\n", player->uid());
    return 0;
}

int Game::PlayerLogIn(const std::string &message, std::shared_ptr<Player> player)
{
    if (player->LogIn(message))
    {
        //PlayerLogOut(player);
        return 1;
    }
    else
    {
        if (uid_players_.find(player->uid()) != uid_players_.end())
        {
            //todo 查看是否在游戏中
        }
        else
        {
            uid_players_[player->uid()] = player;
        }
    }

    return 0;
}

std::shared_ptr<Player> Game::GetPlayByFd(int fd)
{
    auto iter = fd_players_.find(fd);
    if (iter == fd_players_.end())
    {
        return nullptr;
    }
    else
    {
        return iter->second;
    }
}

int Game::InitDb()
{
    int ret = mysql_.Init("127.0.0.1", "root", "123456", "uno");
    if (ret)
    {
        kLog.Error("mysql init error. error_no: %d\n", ret);
    }

    return ret;
}

int Game::ExcuteCmd(int fd, int cmd, const std::string &message)
{
    auto player = GetPlayByFd(fd);
    if (player == nullptr)
    {
        kLog.Error("no this player. cmd:%d\n", cmd);
        return -1;
    }

    kLog.Info("recv cmd:%d\n", cmd);

    int ret = 0;
    switch (cmd)
    {
    case uno::Exchang_CmdList_LOGIN:
        ret = PlayerLogIn(message, player);
    case uno::Exchang_CmdList_LOGOUT:
        break;
    default:
        kLog.Error("cmd:%d invaild.\n", cmd);
        return -2;
    }

    if (ret)
    {
        uno::Error pb_error;
        pb_error.set_code(error_no());
        player->AddSendMessage(uno::Exchang_CmdList_ERROR, pb_error);
    }
    else
    {
        uno::Error pb_error;
        pb_error.set_code(uno::Error_ErrorCode_CORRECT);
        player->AddSendMessage(uno::Exchang_CmdList_ERROR, pb_error);
    }

    return 0;
}

int Game::GetAccountInfo(std::string account, DbAccountInfo &info)
{
    return mysql_.GetAccountInfo(account, info);
}

int Game::GetInfoInfo(int uid, DbInfoInfo &info)
{
    return mysql_.GetInfoInfo(uid, info);
}
