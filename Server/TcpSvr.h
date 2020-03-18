#pragma once

#include <memory>

#include "Manager/ManagerBase.h"
#include "boost/asio.hpp"
#include "Net/NetTcp.h"

using boost::asio::ip::tcp;

class TcpSvr {
public:
    TcpSvr(boost::asio::io_context& io_context,
        int port,
        std::shared_ptr<ManagerBase> ptr)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        manager_ptr_(ptr)
    {
        DoAccept();
    }

    virtual ~TcpSvr()
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
    std::weak_ptr<ManagerBase> manager_ptr_;
};

