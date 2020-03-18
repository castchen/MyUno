#pragma once
#include <string>

class UserBase
{
public:
    virtual ~UserBase() = 0
    {

    }

    virtual void Init() = 0;
    virtual void RecvNetMessage(std::string msg) = 0;
    virtual void Disconnected() = 0;
};

