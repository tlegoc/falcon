//
// Created by theo on 11/02/2025.
//

#include <protocol.h>

Stream::Stream(std::shared_ptr<Falcon> sock, streamid32_t id, bool reliable) :
mLocalSequence(0),
mRemoteSequence(1),
mReliability(reliable),
mStreamID(std::move(id)),
mSocket(sock)
{
}

void Stream::SendData(std::span<const char> Data)
{

}

void Stream::OnDataReceived(std::span<const char> Data)
{

}
