#pragma once

#include <iostream>
#include <set>
#include <memory>

#include "ManagerBase.h"
#include "User/ChatUser.h"
#include "Net/NetBase.h"
#include "Server/ServerTcp.h"
#include "boost/asio.hpp"

class ChatRoom :
    public ManagerBase,
    public std::enable_shared_from_this<ChatRoom>
{
public:
    ChatRoom(boost::asio::io_context& io_context)
        :io_context_(io_context)
    {

    }

    virtual ~ChatRoom()
    {
        std::cout << "~ChatRoom" << std::endl;
    }

    virtual void RecvNewConnect(std::shared_ptr<NetBase> net_ptr) override
    {
        auto user = std::make_shared<ChatUser>(net_ptr, *this);
        user->Init();

        chat_users_set_.insert(user);
    }

    void Init()
    {
        listen_svrs_.insert(std::make_shared<ServerTcp>(io_context_, 1991, shared_from_this()));
    }

    void Deliver(const std::string& msg)
    {
        for (auto e : chat_users_set_)
        {
            e->Deliver(msg);
        }
    }

    void UserDisconnected(std::shared_ptr<ChatUser> user_ptr)
    {
        chat_users_set_.erase(user_ptr);
    }

private:
    std::set<std::shared_ptr<ChatUser>> chat_users_set_;
    std::set<std::shared_ptr<ServerBase>> listen_svrs_;
    boost::asio::io_context& io_context_;
};