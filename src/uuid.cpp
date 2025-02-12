//
// Created by theo on 11/02/2025.
//

#include <uuid.h>

#ifdef _WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif

std::array<char, 37> ToStringArray(const uuid128_t& uuid)
{
    std::array<char, 37> uuidStr; //< Including \0
    std::snprintf(uuidStr.data(), uuidStr.size(), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                  uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
                  uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);

    return uuidStr;
}

std::string ToString(const uuid128_t& uuid)
{
    std::array<char, 37> uuidStr = ToStringArray(uuid);

    return std::string(uuidStr.data(), uuidStr.size() - 1);
}

bool operator==(const uuid128_t& left, const uuid128_t& right)
{
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        if(left[i] != right[i])
            return false;
    }
    return true;
}

bool operator!=(const uuid128_t& left, const uuid128_t& right)
{
    return !(left == right);
}

uuid128_t UuidGenerator::Generate()
{
    std::array<uint8_t,16> uuid {};

#ifdef _WIN32
    GUID id;
    CoCreateGuid(&id);

    for(unsigned int i = 0; i < 4; ++i)
        uuid[i] = static_cast<uint8_t>(id.Data1 >> ((3 - i) * 8 & 0xFF));

    for(unsigned int i = 0; i < 2; ++i)
        uuid[4 + i] = static_cast<uint8_t>(id.Data2 >> ((1 - i) * 8 & 0xFF));

    for(unsigned int i = 0; i < 2; ++i)
        uuid[6 + i] = static_cast<uint8_t>(id.Data3 >> ((1 - i) * 8 & 0xFF));

    for(unsigned int i = 0 ; i < 8; ++i)
        uuid[8 + i] = static_cast<uint8_t>(id.Data4[i]);
#else
    uuid_t id;
    uuid_generate(id);

    std::copy(std::begin(id), std::end(id), uuid.begin());
#endif

    return uuid;
}