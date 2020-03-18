#pragma once

#include <memory>
#include <Manager/ManagerBase.h>

class ServerBase
{
public:
    ServerBase(std::shared_ptr<ManagerBase> manager_ptr)
        : manager_ptr_(manager_ptr)
    {

    }

protected:
    std::weak_ptr<ManagerBase> manager_ptr_;
};
