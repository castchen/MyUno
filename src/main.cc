#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <memory>
#include <unistd.h>
#include <fstream>

#include "log/log.h"
#include "game.h"
#include "./proto/uno.pb.h"
#include "player.h"
#include "./web_socket/WebSocket.h"
#include "third/json.hpp"

void SetSocket(int fd);
void SetNonblockingMode(int fd);
void ForceLogOutPlayer(int clnt_sock);
int CheckRecvFromClient(int recv_len, int clnt_sock);
void ConnectNewPlayer(int serv_sock);
void ReadFd(int clnt_sock);
void WriteFd(int clnt_sock);

Log kLog;
Game kGame;

int main()
{
    std::ifstream uno_conf_file("./conf/uno_conf.json");
    nlohmann::json uno_conf;
    uno_conf_file >> uno_conf;
    
    if (uno_conf["log"]["level"].is_null() || uno_conf["log"]["file"].is_null())
    {
        kLog.Error("log conf error.\n");
        return -1;
    }
    kLog.set_log_level(uno_conf["log"]["level"]);
    kLog.set_file_name(uno_conf["log"]["file"]);

    if (uno_conf["mysql"]["ip"].is_null() || uno_conf["mysql"]["account"].is_null() ||
        uno_conf["mysql"]["passwd"].is_null() || uno_conf["mysql"]["db"].is_null())
    {
        kLog.Error("mysql conf error.\n");
        return -1;
    }
    if (kGame.InitDb(uno_conf["mysql"]["ip"], uno_conf["mysql"]["account"], uno_conf["mysql"]["passwd"], uno_conf["mysql"]["db"]))
    {
        return 0;
    }

    if (!uno_conf["port"].is_number_unsigned())
    {
        kLog.Error("port conf error.\n");
        return -1;
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
    serv_adr.sin_port = htons(uno_conf["port"]);

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

    kLog.Info("game start!\n");
    int event_cnt = 0;
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
                    ReadFd(ep_events[i].data.fd);
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

int CheckRecvFromClient(int recv_len, int clnt_sock)
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
            return -2;
        }
        else
        {
            kLog.Error("recv_len error, auto logout. errno:%d\n", errno);
            ForceLogOutPlayer(clnt_sock);
            return -3;
        }
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

void ReadFd(int clnt_sock)
{
    auto player = kGame.GetPlayByFd(clnt_sock);
    if (player == nullptr)
    {
        kLog.Error("no this player. clnt_sock:%d\n", clnt_sock);
        return;
    }

    int recv_len = 0;
    RecvMessageStruct* recv_message = player->mutable_recv_net_msg();
    Cast::WebSocket* ws = player->mutable_ws();

    while (1)
    {
        recv_len = read(clnt_sock, recv_message->message_ + recv_message->recv_len_, recv_message->max_recv_len_ - recv_message->recv_len_);
        if (CheckRecvFromClient(recv_len, clnt_sock))
        {
            break;
        }
        recv_message->recv_len_ += recv_len;
        recv_message->message_[recv_message->recv_len_] = '\0';

        if (player->status() == Player::Status::kConnected)
        {
            if (ws->UnpackageHandshake(recv_message->message_, recv_message->recv_len_))
            {
                ForceLogOutPlayer(clnt_sock);
                return;
            }

            std::string ws_str;
            ws->PackageHandshake(ws_str);
            write(clnt_sock, ws_str.c_str(), ws_str.length());
            player->set_status(Player::Status::kHandshake);

            recv_message->Reset();
        }
        else if (player->status() > Player::Status::kConnected)
        {
            Cast::WebSocket::UnpackageFrameReturn ws_return;
            if (ws->UnpackageFrame(recv_message->message_, recv_message->recv_len_, ws_return))
            {
                break;
            }

            if (ws_return.m_IsFinal)
            {
                uno::pb::Exchang exchang;
                if (exchang.ParseFromString(ws_return.m_Str))
                {
                    kGame.ExcuteCmd(clnt_sock, exchang.cmd(), exchang.mes());
                }
                else
                {
                    kLog.Error("uno::Exchang parse error.\n");
                }
            }
            else
            {
                kLog.Error("frame is not final.\n");
            }

            if (ws_return.m_FrameLen < recv_message->recv_len_)
            {
                memcpy(recv_message->message_, recv_message->message_ + ws_return.m_FrameLen, recv_message->recv_len_ - ws_return.m_FrameLen);
                recv_message->recv_len_ -= ws_return.m_FrameLen;
            }
            else
            {
                recv_message->Reset();
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
        player->SendNetMessage();
    }
}