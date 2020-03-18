#include "ChatUser.h"
#include "Manager/ChatRoom.h"

void ChatUser::RecvNetMessage(std::string msg)
{
    room_.Deliver(msg);
}

void ChatUser::Disconnected()
{
    room_.UserDisconnected(shared_from_this());
}
