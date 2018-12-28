#include <unistd.h>

#include "player.h"
#include "mysql_db.h"
#include "game.h"
#include <sys/epoll.h>

extern Log kLog;
extern Game kGame;

Player::~Player()
{
    kLog.Info("player delete.\n");
}

int Player::LogOut(std::vector<int> &fd_vec)
{
    int fd = socket_fd();
    if (fd != -1)
    {
        close(fd);
        fd_vec.push_back(fd);
    }
    return 0;
}

int Player::LogIn(const std::string &message)
{
    if (status() >= kHasLogged)
    {
        kLog.Error("Player Log Error. status:%d\n", status());
        return -1;
    }

    uno::LogIn pb_log_in;
    if (!pb_log_in.ParseFromString(message))
    {
        kLog.Error("log in parse error\n");
        return -2;
    }

    DbAccountInfo account;
    if (kGame.GetAccountInfo(pb_log_in.account(), account))
    {
        kLog.Error("account don't find in mysql.\n");
        kGame.set_error_no(uno::Error_ErrorCode_ACCOUNT_OR_PASSWD_ERROR);
        return -4;
    }
   
    if (pb_log_in.account() == account.account_ && pb_log_in.passwd() == account.passwd_)
    {
        DbInfoInfo info;
        if (kGame.GetInfoInfo(account.uid_, info))
        {
            kLog.Error("info don't find in mysql.\n");
            kGame.set_error_no(uno::Error_ErrorCode_ACCOUNT_OR_PASSWD_ERROR);
            return -3;
        }
        else
        {
            set_uid(info.uid_);
            set_name(info.name_);
            set_score(info.score_);
            set_status(kHasLogged);
            kLog.Info("uid: %d name: %s score: %d login success.\n", uid(), name().c_str(), score());
        }
    }
    else
    {
        kLog.Error("account or passwd error.\n");
        kGame.set_error_no(uno::Error_ErrorCode_ACCOUNT_OR_PASSWD_ERROR);
        return -4;
    }

    return 0;
}

void Player::SendMessage()
{
    if (!send_message_.empty())
    {
        auto message = send_message_.front();
        uint32_t length = message.length();
        write(socket_fd(), &length, sizeof(uint32_t));
        write(socket_fd(), message.c_str(), length);
        send_message_.pop();
    }

    if (send_message_.empty())
    {
        SetSocketFdRead();
    }
}

void Player::SetSocketFdReadAndWrite()
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.fd = socket_fd();
    epoll_ctl(kGame.ep_fd(), EPOLL_CTL_MOD, socket_fd(), &event);
}

void Player::SetSocketFdRead()
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = socket_fd();
    epoll_ctl(kGame.ep_fd(), EPOLL_CTL_MOD, socket_fd(), &event);
}
