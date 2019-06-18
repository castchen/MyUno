#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <string>
#include <vector>
#include <queue>

#include "log/log.h"
#include "proto/uno.pb.h"
#include "./web_socket/WebSocket.h"
#include "cmd_list.h"

constexpr int kBufSize = 1024 * 30;

struct RecvMessageStruct
{
    uint64_t recv_len_ = 0;
    uint64_t max_recv_len_ = kBufSize - 1;
    char message_[kBufSize];

    void Reset()
    {
        recv_len_ = 0;
        max_recv_len_ = kBufSize - 1;
        memset(&message_, 0, sizeof(message_));
    }
};

struct SendMessageStruct
{
    uint64_t send_len_ = 0;
    uint64_t max_send_len_ = 0;
    char* message_ = nullptr;

    SendMessageStruct() = default;

    ~SendMessageStruct()
    {
        if (message_)
        {
            delete[] message_;
            message_ = nullptr;
        }
    }
};

class Player
{
public:
    enum Status
    {
        kInit,
        kConnected,
        kHandshake,
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

    RecvMessageStruct* mutable_recv_net_msg() { return &recv_net_msg_; }

    Cast::WebSocket* mutable_ws() { return &ws_; }

public:
    Player() = default;

    Player(int fd, Status status) :
        socket_fd_(fd), status_(status)
    {

    }

    ~Player();

    int LogOut(std::vector<int> &fd_vec);

    int LogIn(const std::string &message);

    void SendNetMessage();

    int CheckSendLen(int send_len, SendMessageStruct &message);

    template <typename T> void AddSendMessage(CmdList cmd, const T &pb_message);

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
    RecvMessageStruct   recv_net_msg_;
    Cast::WebSocket     ws_;
};

template <typename T> void Player::AddSendMessage(CmdList cmd, const T &pb_message)
{
    std::string pb_string;
    pb_message.SerializeToString(&pb_string);

    uno::pb::Exchang pb_exchange;
    pb_exchange.set_cmd(cmd);
    pb_exchange.set_mes(pb_string);

    pb_string.clear();
    pb_exchange.SerializeToString(&pb_string);

    SendMessageStruct message;
    ws_.PackageTextFrame(pb_string, message.message_, message.max_send_len_);

    if (message.message_)
    {
        wait_send_message_.push(message);
    }

    SendNetMessage();
}

#endif