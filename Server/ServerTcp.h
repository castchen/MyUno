#pragma once

#include <memory>

#include "ServerBase.h"
#include "Manager/ManagerBase.h"
#include "boost/asio.hpp"
#include "Net/NetTcp.h"

using boost::asio::ip::tcp;

class ServerTcp :
    public ServerBase
{
public:
    ServerTcp(boost::asio::io_context& io_context,
        int port,
        std::shared_ptr<ManagerBase> ptr)
        : ServerBase(ptr), acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        DoAccept();
    }

    virtual ~ServerTcp()
    {
        acceptor_.close();
    }

private:
    void DoAccept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    auto manager = manager_ptr_.lock();
                    if (manager)
                    {
                        auto net_tcp = std::make_shared<NetTcp>(std::move(socket));
                        manager->RecvNewConnect(net_tcp);
                    }
                }

                DoAccept();
            }
        );
    }

private:
    tcp::acceptor acceptor_;
};

