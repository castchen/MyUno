#include <iostream>
#include <memory>

#include "Manager/ChatRoom.h"
#include "boost/asio.hpp"

int main()
{
    boost::asio::io_context io_context;
    auto chat_room = std::make_shared<ChatRoom>(io_context);
    chat_room->Init();
    io_context.run();

    return 0;
}

