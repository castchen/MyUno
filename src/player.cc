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

    uno::pb::ReqLogIn pb_log_in;
    if (!pb_log_in.ParseFromString(message))
    {
        kLog.Error("log in parse error\n");
        return -2;
    }

    DbAccountInfo account;
    if (kGame.GetAccountInfo(pb_log_in.account(), account))
    {
        kLog.Error("account don't find in mysql.\n");
        kGame.set_error_no(RetCodeList::kRetCodeAccountOrPasswdError);
        return -4;
    }
   
    if (pb_log_in.account() == account.account_ && pb_log_in.passwd() == account.passwd_)
    {
        DbInfoInfo info;
        if (kGame.GetInfoInfo(account.uid_, info))
        {
            kLog.Error("info don't find in mysql.\n");
            kGame.set_error_no(RetCodeList::kRetCodeAccountOrPasswdError);
            return -3;
        }
        else
        {
            set_uid(info.uid_);
            set_name(info.name_);
            set_score(info.score_);
            set_status(kHasLogged);
            kLog.Info("uid: %d name: %s score: %d login success.\n", uid(), name().c_str(), score());

            uno::pb::AckLogIn pb_ack;
            pb_ack.set_uid(uid());
            pb_ack.set_name(name());
            pb_ack.set_score(score());
            AddSendMessage(CmdList::kCmdLogin, pb_ack);
        }
    }
    else
    {
        kLog.Error("account or passwd error.\n");
        kGame.set_error_no(RetCodeList::kRetCodeAccountOrPasswdError);
        return -4;
    }

    return 0;
}

void Player::SendNetMessage()
{
    int send_len = 0;
    while (!wait_send_message_.empty())
    {
        SendMessageStruct &message = wait_send_message_.front();
        send_len = write(socket_fd(), message.message_ + message.send_len_, message.max_send_len_ - message.send_len_);
        if (CheckSendLen(send_len, message))
        {
            break;
        }

        wait_send_message_.pop();
    }

    if (!wait_send_message_.empty())
    {
        if (!send_buff_full_)
        {
            SetSocketFdReadAndWrite();
            send_buff_full_ = true;
        }
    }
    else
    {
        if (send_buff_full_)
        {
            SetSocketFdRead();
            send_buff_full_ = false;
        }
    }
}

int Player::CheckSendLen(int send_len, SendMessageStruct &message)
{
    if (send_len < 0)
    {
        if (errno == EAGAIN)
        {
            kLog.Error("SendBuf is full. uid:%d errno:%d \n", uid(), errno);
        }
        else
        {
            kLog.Error("SendLen Error. uid:%d errno:%d \n", uid(), errno);
        }
        return -1;
    }

    message.send_len_ += send_len;

    if (message.send_len_ > message.max_send_len_)
    {
        kLog.Error("Player Send bigger max. len:%d max:%d\n", message.send_len_, message.max_send_len_);
        return -2;
    }
    else if (message.send_len_ < message.max_send_len_)
    {
        kLog.Error("Player Send less max. len:%d max:%d\n", message.send_len_, message.max_send_len_);
        return -3;
    }

    return 0;
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
