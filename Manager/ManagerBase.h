#pragma once

class NetBase;

class ManagerBase
{
public:
    virtual ~ManagerBase() = 0
    {

    }

    virtual void RecvNewConnect(std::shared_ptr<NetBase> net_ptr) = 0;
};
