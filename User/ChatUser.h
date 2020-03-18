#pragma once

#include <memory>
#include <iostream>

#include "UserBase.h"
#include "Net/NetBase.h"

class ChatRoom;

class ChatUser :
    public UserBase,
    public std::enable_shared_from_this<ChatUser>
{
public:
    ChatUser(std::shared_ptr<NetBase> net_ptr, ChatRoom& room) :
        net_ptr_(net_ptr), room_(room)
    {}

    virtual ~ChatUser()
    {
        std::cout << "~ChatUser" << std::endl;
    }

    virtual void Init()
    {
        net_ptr_->Init(shared_from_this());
    }

    virtual void RecvNetMessage(std::string msg) override;

    virtual void Disconnected() override;

    void Deliver(const std::string& msg)
    {
        net_ptr_->SendNetMessage(msg);
    }

private:
    std::shared_ptr<NetBase> net_ptr_;
    ChatRoom& room_;
};

