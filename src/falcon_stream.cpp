//
// Created by theo on 11/02/2025.
//

#include <protocol.h>

Stream::Stream(std::shared_ptr<Falcon> sock, streamid32_t id, bool reliable) :
mLocalSequence(0),
mRemoteSequence(0xFFFF),
mReliability(reliable),
mStreamID(std::move(id)),
mSocket(sock)
{
}

void Stream::SendData(std::span<const char> Data)
{

    mLocalSequence++;
    // Vérifier la taille de la data qu'on veut envoyer
    // si la taille est superieure au MTU, on fragmente
    // Rajouter le header de fragmentation
    // Rajouter l'historique de ACK dans le paquet
}

void Stream::OnDataReceived(std::span<const char> Data)
{

}
