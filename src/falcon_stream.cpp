//
// Created by theo on 11/02/2025.
//

#include <protocol.h>

streamid32_t Stream::streamCounter = 0;

Stream::Stream(std::shared_ptr<Falcon> sock, uuid128_t client, bool reliable) :
mLocalSequence(0),
mRemoteSequence(0xFFFF),
mReliability(reliable),
mSocket(std::move(sock)),
clientID(client),
mAckHistory()
{
    mStreamID = streamCounter++;
}

void Stream::SendData(std::span<const char> data)
{
    // Vérifier la taille de la data qu'on veut envoyer
    size_t packetFrags = std::ceil(data.size() / MTU);
    if(packetFrags < 1)
    {
        for(size_t i = 0; i < packetFrags; ++i)
        {

        }
    } else {
        //Construction du packet
        PacketBuilder packet(PacketType::DATA);
        DataHeader dataHeader{
            clientID,
            mStreamID,
            mLocalSequence,
            data.size(),
            0
        };
        packet.AddStruct(dataHeader);
        packet.AddData(data);

        // Rajouter le packet dans la liste d'attente
        mAckWaitList.push_back(StreamPacket{mLocalSequence, packet.GetData()});

        //Envoyer le packet
//        mSocket->SendTo();
        ++mLocalSequence;
    }


}

void Stream::OnDataReceived(std::span<const char> data)
{

}