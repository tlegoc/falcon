//
// Created by theo on 11/02/2025.
//

#include <protocol.h>
#include <spdlog/spdlog.h>

Stream::Stream(IStreamProvider *streamProvider, uuid128_t client, bool reliable) :
        mLocalSequence(0),
        mRemoteSequence(0xFFFF),
        mReliability(reliable),
        mStreamProvider(std::move(streamProvider)),
        mClientID(client),
        mAckHistory() {
    mStreamID = UuidGenerator::Generate();

    // Should be one bit only but the big flemme (TM)
    if (reliable) {
        mStreamID[0] = 1;
    } else {
        mStreamID[0] = 0;
    }
}

void Stream::SendData(std::span<const char> data)
{
//    spdlog::debug("Sending data to stream {}, size: {}", ToString(mStreamID), data.size());

    // Vérifier la taille de la data qu'on veut envoyer
    float adjustedMTU = static_cast<float>(MTU) - sizeof(DataSplitHeader) - sizeof(DataHeader);
    size_t packetFrags = std::ceil(static_cast<float>(data.size()) / adjustedMTU);

    if (packetFrags > 1)
    {
        for (size_t i = 0; i < packetFrags; ++i)
        {
            // Calcul de la taille de la data restante
            size_t offset = i * MTU;
            size_t length = std::min(static_cast<size_t>(MTU), data.size() - offset);

            // On cree le packet
            PacketBuilder packetBuilder(PacketType::DATA);
            DataHeader dataHeader{
                    mClientID,
                    mStreamID,
                    mLocalSequence,
                    length,
                    1
            };
            packetBuilder.AddStruct(dataHeader);

            DataSplitHeader dataSplitHeader{
                    static_cast<uint32_t>(i),
                    static_cast<uint32_t>(packetFrags)
            };
            packetBuilder.AddStruct(dataSplitHeader);

            // Copier le paquet depuis offset jusqu'à length
            packetBuilder.AddData(&data[offset], length);

            auto packet = packetBuilder.GetData();
            // Ajouter le packet a la liste d'attente
            if (mReliability)
                mAckWaitList.push_back(StreamPacket(mLocalSequence, packet));

            // Envoyer le packet (fragmenté)
            mStreamProvider->SendStreamPacket(mClientID, packet);
        }
    } else {
        //Construction du packet
        PacketBuilder packetBuilder(PacketType::DATA);
        DataHeader dataHeader{
                mClientID,
                mStreamID,
                mLocalSequence,
                data.size(),
                DataFlag::NONE
        };
        packetBuilder.AddStruct(dataHeader);
        packetBuilder.AddData(data);

        auto packet = packetBuilder.GetData();
        // Rajouter le packet dans la liste d'attente
        if (mReliability)
            mAckWaitList.push_back(StreamPacket{mLocalSequence, packet});

        // Envoyer le packet
        mStreamProvider->SendStreamPacket(mClientID, packet);
    }
    ++mLocalSequence;
}

void Stream::SendMissingPackets(const std::vector<uint32_t> &ackedList)
{

}

void Stream::HandlePartialPacket(uint32_t packetId, DataSplitHeader header, std::span<const char> packetData)
{
    mReceivedFragmentPacket[packetId].total = header.total;
    mReceivedFragmentPacket[packetId].ids.push_back(header.partId);
    mReceivedFragmentPacket[packetId].data.push_back(packetData);
    // Check l'id du packet et le total

    // Verifier si on a le total
    // Si on a le total reconstruire le packet et appeler OnDataReceived avec
}

void Stream::OnDataReceived(std::function<void(std::span<const char>)> function) {
    mDataReceivedHandler = std::move(function);
}

void Stream::HandleDataReceived(const std::span<const char> data) {
    mDataReceivedHandler(data);
}