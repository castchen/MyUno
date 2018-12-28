#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <string>
#include <vector>
#include <queue>

#include "log/log.h"
#include "proto/uno.pb.h"

class Player {
public:
    enum Status
    {
        kInit,
        kConnected,
        kHasLogged,
        kInTheGame,
    };

    int socket_fd() const { return socket_fd_; }

    void set_socket_fd(int val) { socket_fd_ = val; }

    int uid() const { return uid_; }

    void set_uid(int val) { uid_ = val; }

    Player::Status status() const { return status_; }

    void set_status(Player::Status val) { status_ = val; }

    std::string name() const { return name_; }

    void set_name(std::string val) { name_ = val; }

    int score() const { return score_; }

    void set_score(int val) { score_ = val; }

public:
    Player() = default;
    
    Player(int fd, Status status) :
        socket_fd_(fd), status_(status)
    {

    }

    ~Player();

    int LogOut(std::vector<int> &fd_vec);

    int LogIn(const std::string &message);

    void SendMessage();

    template <typename T>
    void AddSendMessage(uno::Exchang_CmdList cmd, const T &pb_message);

    void SetSocketFdReadAndWrite();

    void SetSocketFdRead();

private:
    int                 socket_fd_ = -1;
    int                 uid_ = -1;
    std::string         name_;
    int                 score_ = 0;
    Status              status_ = kInit;
    std::queue<std::string> send_message_;
};

template <typename T>
void Player::AddSendMessage(uno::Exchang_CmdList cmd, const T &pb_message)
{
    std::string pb_string;
    pb_message.SerializeToString(&pb_string);

    uno::Exchang pb_exchange;
    pb_exchange.set_cmd(cmd);
    pb_exchange.set_mes(pb_string);

    pb_string.clear();
    pb_exchange.SerializeToString(&pb_string);
    send_message_.push(pb_string);

    SetSocketFdReadAndWrite();
}

#endif