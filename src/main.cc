#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <memory>
#include <unistd.h>

#include "log/log.h"
#include "game.h"
#include "./proto/uno.pb.h"

constexpr int kBufSize = 1024 * 10;

struct RecvMessageStruct
{
    enum Status
    {
        kRecvLength,
        kRecvMessage,
    };
    int recv_len_ = 0;
    int max_recv_len_ = 0;
    char message_[kBufSize];
    Status status_ = kRecvLength;

    void Reset()
    {
        recv_len_ = 0;
        max_recv_len_ = 0;
        memset(&message_, 0, sizeof(message_));
        status_ = kRecvLength;
    }

    void SetRecvLength()
    {
        recv_len_ = 0;
        max_recv_len_ = 4;
        memset(&message_, 0, sizeof(message_));
        status_ = kRecvLength;
    }

    void SetRecvMessage(int length)
    {
        recv_len_ = 0;
        max_recv_len_ = length;
        memset(&message_, 0, sizeof(message_));
        status_ = kRecvMessage;
    }
};

void SetSocket(int fd);
void SetNonblockingMode(int fd);
void ForceLogOutPlayer(int clnt_sock);
int CheckRecvFromClient(int recv_len, int clnt_sock, RecvMessageStruct &recv_message, 
    char *buf, std::map<int, RecvMessageStruct> &fd_message);
void ConnectNewPlayer(int serv_sock);
void ReadFd(int clnt_sock, std::map<int, RecvMessageStruct> &fd_message);
void WriteFd(int clnt_sock);

Log kLog;
Game kGame;

int main()
{
    kLog.set_log_level(Log::LOG_DEBUG);
    kLog.set_file_name("./log/uno.log");

    if (kGame.InitDb())
    {
        return 0;
    }

    int serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        kLog.Error("socket error.\n");
        return -1;
    }
    SetSocket(serv_sock);
    SetNonblockingMode(serv_sock);

    struct sockaddr_in serv_adr;
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(6666);

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        kLog.Error("bind error.\n");
        return -1;
    }
    if (listen(serv_sock, 5) == -1)
    {
        kLog.Error("listen error.\n");
        return -1;
    }

    const int kEpSize = 50;
    kGame.set_ep_fd(epoll_create(kEpSize));

    struct epoll_event *ep_events = static_cast<struct epoll_event *> (malloc(sizeof(struct epoll_event) * kEpSize));

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(kGame.ep_fd(), EPOLL_CTL_ADD, serv_sock, &event);

    int event_cnt = 0;
    std::map<int, RecvMessageStruct> fd_message;
    while (1)
    {
        event_cnt = epoll_wait(kGame.ep_fd(), ep_events, kEpSize, -1);
        if (event_cnt == -1)
        {
            kLog.Error("epoll -1.\n");
            break;
        }
        for (int i = 0; i < event_cnt; ++i)
        {
            if (ep_events[i].data.fd == serv_sock)
            {
                ConnectNewPlayer(serv_sock);
            }
            else
            {
                if (ep_events[i].events & EPOLLIN)
                {
                    ReadFd(ep_events[i].data.fd, fd_message);
                }
                if (ep_events[i].events & EPOLLOUT)
                {
                    WriteFd(ep_events[i].data.fd);
                }
            }
        }
    }

    close(serv_sock);
    close(kGame.ep_fd());

    return 0;
}

void SetNonblockingMode(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void SetSocket(int fd)
{
    int option = 1;
    int optlen = sizeof(option);
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);
}

int CheckRecvFromClient(int recv_len, int clnt_sock, RecvMessageStruct &recv_message,
    char *buf, std::map<int, RecvMessageStruct> &fd_message)
{
    if (recv_len == 0)
    {
        ForceLogOutPlayer(clnt_sock);
        return -1;
    }
    else if (recv_len < 0)
    {
        if (errno == EAGAIN)
        {
        }
        else
        {
            kLog.Error("recv_len error, auto logout. errno:%d\n", errno);
            ForceLogOutPlayer(clnt_sock);
            return -2;
        }
    }
    else
    {
        memcpy(recv_message.message_ + recv_message.recv_len_, buf, recv_len);
        recv_message.recv_len_ += recv_len;
    }

    if (recv_message.recv_len_ < recv_message.max_recv_len_)
    {
        fd_message[clnt_sock] = recv_message;
        return -3;
    }

    return 0;
}

void ForceLogOutPlayer(int clnt_sock)
{
    epoll_ctl(kGame.ep_fd(), EPOLL_CTL_DEL, clnt_sock, NULL);
    auto player = kGame.GetPlayByFd(clnt_sock);
    kGame.PlayerLogOut(player);
}

void ConnectNewPlayer(int serv_sock)
{
    struct sockaddr_in clnt_adr;
    socklen_t adr_sz = sizeof(clnt_adr);

    while (1)
    {
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);

        if (clnt_sock == -1)
        {
            if (errno == EAGAIN)
            {
                return;
            }
            kLog.Error("accept error.\n");
            return;
        }
        auto player = std::make_shared<Player>(clnt_sock, Player::kConnected);
        if (kGame.PlayerConnect(clnt_sock, player) < 0)
        {
            kGame.PlayerLogOut(player);
        }
        else
        {
            struct epoll_event event;
            SetNonblockingMode(clnt_sock);
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = clnt_sock;
            epoll_ctl(kGame.ep_fd(), EPOLL_CTL_ADD, clnt_sock, &event);
            kLog.Info("connected client:%d ip:%s port:%d\n", clnt_sock, inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));
        }
    }
}

void ReadFd(int clnt_sock, std::map<int, RecvMessageStruct> &fd_message)
{
    char buf[kBufSize];
    int recv_len = 0;
    RecvMessageStruct recv_message;

    while (1)
    {
        if (fd_message.find(clnt_sock) == fd_message.end())
        {
            recv_message.SetRecvLength();
        }
        else
        {
            recv_message = fd_message[clnt_sock];
        }

        if (recv_message.status_ == RecvMessageStruct::kRecvLength)
        {
            recv_len = read(clnt_sock, buf, recv_message.max_recv_len_ - recv_message.recv_len_);
            buf[recv_len] = '\0';

            if (CheckRecvFromClient(recv_len, clnt_sock, recv_message, buf, fd_message))
            {
                break;
            }

            uint32_t message_len = 0;
            memcpy(&message_len, recv_message.message_, sizeof(uint32_t));
            if (message_len >= kBufSize)
            {
                kLog.Error("will recv message length too long. len: %d\n", message_len);
                ForceLogOutPlayer(clnt_sock);
                break;
            }

            recv_message.SetRecvMessage(message_len);
        }

        kLog.Debug("fd:%d cur_length: %d total_length: %d\n", clnt_sock, recv_message.recv_len_, recv_message.max_recv_len_);

        if (recv_message.status_ == RecvMessageStruct::kRecvMessage)
        {
            recv_len = read(clnt_sock, buf, recv_message.max_recv_len_ - recv_message.recv_len_);
            buf[recv_len] = '\0';

            if (CheckRecvFromClient(recv_len, clnt_sock, recv_message, buf, fd_message))
            {
                break;
            }

            uno::Exchang exchang;
            exchang.ParseFromArray(recv_message.message_, recv_message.max_recv_len_);
            kGame.ExcuteCmd(clnt_sock, exchang.cmd(), exchang.mes());

            auto iter = fd_message.find(clnt_sock);
            if (iter != fd_message.end())
            {
                fd_message.erase(iter);
            }
        }
    }
}

void WriteFd(int clnt_sock)
{
    kLog.Debug("In WriteFd\n");
    auto player = kGame.GetPlayByFd(clnt_sock);
    if (player)
    {
        player->SendMessage();
    }
}