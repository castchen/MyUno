#pragma once

#include <utility>
#include <memory>
#include <deque>
#include <iostream>

#include "NetBase.h"
#include "boost/asio.hpp"
#include "User/UserBase.h"

using boost::asio::ip::tcp;

class TcpMessage
{
public:
    enum { header_length = 4 };
    enum { max_body_length = 512 };

    TcpMessage()
        : body_length_(0)
    {}

    const char* data() const
    {
        return data_;
    }

    char* data()
    {
        return data_;
    }

    std::size_t length() const
    {
        return header_length + body_length_;
    }

    const char* body() const
    {
        return data_ + header_length;
    }

    char* body()
    {
        return data_ + header_length;
    }

    std::size_t body_length() const
    {
        return body_length_;
    }

    void body_length(std::size_t new_length)
    {
        body_length_ = new_length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;
    }

    bool decode_header()
    {
        char header[header_length + 1] = "";
        std::strncat(header, data_, header_length);
        body_length_ = std::atoi(header);
        if (body_length_ > max_body_length)
        {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void encode_header()
    {
        char header[header_length + 1] = "";
        std::sprintf(header, "%4d", static_cast<int>(body_length_));
        std::memcpy(data_, header, header_length);
    }

private:
    char data_[header_length + max_body_length];
    std::size_t body_length_;
};

typedef std::deque<TcpMessage> TcpMessageQueue;

class NetTcp :
    public NetBase,
    public std::enable_shared_from_this<NetTcp>
{
public:
    NetTcp(tcp::socket socket) 
        : socket_(std::move(socket))
    {

    }

    virtual ~NetTcp()
    {
        socket_.close();
        std::cout << "~NetTcp" << std::endl;
    }

    virtual void Init(std::shared_ptr<UserBase> user) override
    {
        NetBase::Init(user);
        DoReadHeader();
    }

    virtual void SendNetMessage(const std::string& msg) override
    {
        TcpMessage tcp_msg;
        tcp_msg.body_length(msg.length());
        std::memcpy(tcp_msg.body(), msg.c_str(), tcp_msg.body_length());
        tcp_msg.encode_header();

        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(tcp_msg);
        if (!write_in_progress)
        {
            DoWrite();
        }
    }

private:
    void DoReadHeader()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.data(), TcpMessage::header_length),
            [this, self](boost::system::error_code ec, std::size_t)
            {
                if (!ec && read_msg_.decode_header())
                {
                    DoReadBody();
                }
                else if(ec != boost::asio::error::operation_aborted)
                {
                    auto user = user_ptr_.lock();
                    if (user)
                    {
                        user->Disconnected();
                    }
                }
            }
        );
    }

    void DoReadBody()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
            [this, self](boost::system::error_code ec, std::size_t)
            {
                if (!ec)
                {
                    auto user = user_ptr_.lock();
                    if (user)
                    {
                        std::string msg(read_msg_.body(), read_msg_.body_length());
                        user->RecvNetMessage(std::move(msg));
                    }
                    DoReadHeader();
                }
                else if (ec != boost::asio::error::operation_aborted)
                {
                    auto user = user_ptr_.lock();
                    if (user)
                    {
                        user->Disconnected();
                    }
                }
            }
        );
    }

    void DoWrite()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
            boost::asio::buffer(write_msgs_.front().data(),
                write_msgs_.front().length()),
            [this, self](boost::system::error_code ec, std::size_t)
            {
                if (!ec)
                {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty())
                    {
                        DoWrite();
                    }
                }
            }
        );
    }

private:
    tcp::socket socket_;
    TcpMessage  read_msg_;
    TcpMessageQueue write_msgs_;
};
