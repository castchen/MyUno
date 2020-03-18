#pragma once

#include <string>

class UserBase;

class NetBase
{
public:
    virtual ~NetBase() = 0
    {

    }

    virtual void SendNetMessage(const std::string& msg) = 0;

    virtual void Init(std::shared_ptr<UserBase> user)
    {
        user_ptr_ = user;
    }

protected:
    std::weak_ptr<UserBase> user_ptr_;
};

