#pragma once

#include <memory>

#include "UserBase.h"
#include "Net/NetBase.h"

class User :
    public UserBase,
    public std::enable_shared_from_this<User>
{
public:
    User(std::shared_ptr<NetBase> net_ptr) :
        net_ptr_(net_ptr)
    {
    }

    virtual void Init()
    {
        net_ptr_->Init(shared_from_this());
    }

    virtual void RecvNetMessage(std::string msg) override
    {

    }

    virtual void Disconnected() override
    {

    }


private:
    std::shared_ptr<NetBase> net_ptr_;
};

