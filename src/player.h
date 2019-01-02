#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <string>
#include <vector>
#include <queue>

#include "log/log.h"
#include "proto/uno.pb.h"

struct SendMessageStruct
{
    enum Status
    {
        kSendLength,
        kSendMessage,
    };

    int send_len_ = 0;
    int max_send_len_ = 0;
    uint32_t length_ = 0;
    std::string message_;
    Status status_ = kSendLength;

    SendMessageStruct() = default;

    SendMessageStruct(uint32_t length, const std::string &message)
        : length_(length), message_(message)
    {
    }

    void SetSendLength()
    {
        send_len_ = 0;
        max_send_len_ = 4;
        status_ = kSendLength;
    }

    void SetSendMessage()
    {
        send_len_ = 0;
        max_send_len_ = message_.length();
        status_ = kSendMessage;
    }
};

class Player
{
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

    int CheckSendLen(int send_len, SendMessageStruct &message);

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
    std::queue<SendMessageStruct> wait_send_message_;
    bool                send_buff_full_ = false;
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

    SendMessageStruct message(pb_string.length(), pb_string);
    message.SetSendLength();

    wait_send_message_.push(message);

    SendMessage();
}

#endif