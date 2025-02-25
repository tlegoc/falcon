//
// Created by theo on 11/02/2025.
//

#include <protocol.h>
#include <spdlog/spdlog.h>

uint32_t Stream::fragmentPacketId = 0;

Stream::Stream(IStreamProvider *streamProvider, uuid128_t client, bool reliable) :
        mLocalSequence(0),
        mReliability(reliable),
        mStreamProvider(std::move(streamProvider)),
        mClientID(client),
        mLastReceivedMessage(0),
        mAckHistory() {
    mStreamID = UuidGenerator::Generate();

    // Should be one bit only but the big flemme (TM)
    if (reliable) {
        mStreamID[0] = 1;
    } else {
        mStreamID[0] = 0;
    }
}

void Stream::SendData(std::span<const char> data) {
//    spdlog::debug("Sending data to stream {}, size: {}", ToString(mStreamID), data.size());

    // Vérifier la taille de la data qu'on veut envoyer
    float adjustedMTU = static_cast<float>(MTU) - sizeof(DataSplitHeader) - sizeof(DataHeader) - sizeof(PacketHeader);
    size_t packetFrags = std::ceil(static_cast<float>(data.size()) / adjustedMTU);

    if (packetFrags > 1) {
        spdlog::debug("Sending {} packets.", packetFrags);

        for (size_t i = 0; i < packetFrags; ++i) {
            // Calcul de la taille de la data restante
            size_t offset = i * adjustedMTU;
            size_t length = std::min(static_cast<size_t>(adjustedMTU), data.size() - offset);

            // On cree le packet
            PacketBuilder packetBuilder(PacketType::DATA);
            DataHeader dataHeader{
                    mClientID,
                    mStreamID,
                    mLocalSequence,
                    length,
                    DataFlag::FRAGMENTED
            };
            packetBuilder.AddStruct(dataHeader);

            DataSplitHeader dataSplitHeader{
                    fragmentPacketId,
                    static_cast<uint32_t>(i),
                    static_cast<uint32_t>(packetFrags)
            };
            packetBuilder.AddStruct(dataSplitHeader);

            // Copier le paquet depuis offset jusqu'à length
            packetBuilder.AddData(data.data() + offset, length);

            auto packet = packetBuilder.GetData();
            // Ajouter le packet a la liste d'attente
            if (mReliability) {
                mAckWaitList[mLocalSequence] = {
                        packet.size()
                };
                packetBuilder.CopyToArray(mAckWaitList[mLocalSequence].data);
            }

            // Envoyer le packet (fragmenté)
            mStreamProvider->SendStreamPacket(mClientID, packet);
            ++mLocalSequence;
        }
        fragmentPacketId++;
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
        if (mReliability) {
            mAckWaitList[mLocalSequence] = {
                    packet.size()
            };
            packetBuilder.CopyToArray(mAckWaitList[mLocalSequence].data);
        }

        // Envoyer le packet
        mStreamProvider->SendStreamPacket(mClientID, packet);
        ++mLocalSequence;
    }
}

void Stream::HandleMissingPackets(const std::vector<uint32_t> &ackedList) {
    for (auto id: ackedList) {
        mAckWaitList.erase(id);
    }

    for (auto &[id, packet]: mAckWaitList) {
        mStreamProvider->SendStreamPacket(mClientID, {packet.data.data(), packet.size});
    }
}

void Stream::HandlePartialPacket(DataSplitHeader header, std::span<const char> packetData, size_t size) {
    mReceivedFragmentPacket[header.splittedMsgId].total = header.total;
    mReceivedFragmentPacket[header.splittedMsgId].sizes[header.partId] = size;
    mReceivedFragmentPacket[header.splittedMsgId].fragment[header.partId].resize(size);
    std::memcpy(mReceivedFragmentPacket[header.splittedMsgId].fragment[header.partId].data(), packetData.data(), size);

    // Verifier si on a le total
    if (mReceivedFragmentPacket[header.splittedMsgId].fragment.size() < header.total)
        return;

    spdlog::debug("Rebuilding splitted packet: {}", header.splittedMsgId);

    // Si on a le total reconstruire le packet et appeler OnDataReceived avec
    std::vector<char> reconstructedPacket{};
    for (size_t i = 0; i < header.total; ++i) {
        size_t arraySize = reconstructedPacket.size();
        size_t dataSize = mReceivedFragmentPacket[header.splittedMsgId].sizes[i];
        reconstructedPacket.resize(arraySize + dataSize);
        std::memcpy(reconstructedPacket.data() + arraySize, mReceivedFragmentPacket[header.splittedMsgId].fragment[i].data(), dataSize);
    }

    // Packet reconstruit, on peut le traiter comme un packet reçu
    mReceivedFragmentPacket.erase(header.splittedMsgId);
    HandleDataReceived(reconstructedPacket);
}

void Stream::OnDataReceived(std::function<void(std::span<const char>)> function) {
    mDataReceivedHandler = std::move(function);
}

void Stream::HandleDataReceived(const std::span<const char> data) {
    mDataReceivedHandler(data);
}

std::vector<uint32_t> Stream::GetReceivedMessagesFromBitfield(uint32_t lastMsg, std::bitset<PROTOCOL_HISTORY_SIZE> bitfield) {
    std::vector<uint32_t> receivedMessages;

    for (int i = 0; i < PROTOCOL_HISTORY_SIZE; i++) {
        if (lastMsg - i > lastMsg) break;

        auto bit = bitfield[i];
        if (bit) {
            receivedMessages.push_back(lastMsg - i);
        }
    }

    return receivedMessages;
}

std::bitset<PROTOCOL_HISTORY_SIZE> Stream::GetBitFieldFromLastReceived(uint32_t lastMsg, std::vector<uint32_t> received) {
    std::bitset<PROTOCOL_HISTORY_SIZE> bitfield;

    for (int i = 0; i < received.size(); i++) {
        auto msg = received[i];

        if (lastMsg - msg > PROTOCOL_HISTORY_SIZE) continue;

        bitfield[lastMsg - msg] = true;
    }

    return bitfield;
}

bool Stream::HandleAck(uint32_t msgId) {
    bool isMsgNew = true;
    if (msgId > mLastReceivedMessage) {
        mAckHistory <<= (msgId - mLastReceivedMessage);
        mLastReceivedMessage = msgId;
        mAckHistory[0] = true;
    } else {
        uint32_t index = mLastReceivedMessage - msgId;

        if (index < PROTOCOL_HISTORY_SIZE) {
            if (mAckHistory[index]) isMsgNew = false;

            mAckHistory[index] = true;
        } else { // Message is too old
            isMsgNew = false;
        }
    }

    PacketBuilder packetBuilder(PacketType::DATA_ACK);

    packetBuilder.AddStruct(DataAckHeader{
            mClientID,
            mStreamID,
            mLastReceivedMessage,
            Stream::BitsetToConstChar(mAckHistory)
    });

    auto packet = packetBuilder.GetData();
    mStreamProvider->SendStreamPacket(mClientID, packet);

    return isMsgNew;
}

std::array<char, PROTOCOL_HISTORY_SIZE/8> Stream::BitsetToConstChar(std::bitset<PROTOCOL_HISTORY_SIZE> bitset) {
    std::array<char, PROTOCOL_HISTORY_SIZE/8> chars{};
    for (size_t i = 0; i < PROTOCOL_HISTORY_SIZE/8; ++i) {
        char c = 0;
        for (size_t bit = 0; bit < 8; ++bit) {
            c |= (bitset[i * 8 + bit] << bit);
        }
        chars[i] = c;
    }
    return chars;
}

std::bitset<PROTOCOL_HISTORY_SIZE> Stream::ConstCharToBitSet(std::array<char, PROTOCOL_HISTORY_SIZE/8> chars) {
    std::bitset<PROTOCOL_HISTORY_SIZE> bitset;
    for (size_t i = 0; i < PROTOCOL_HISTORY_SIZE/8; ++i) {
        for (size_t bit = 0; bit < 8; ++bit) {
            bitset[i * 8 + bit] = (chars[i] >> bit) & 1;
        }
    }
    return bitset;
}
