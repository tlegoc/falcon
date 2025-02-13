//
// Created by theo on 11/02/2025.
//

#pragma once

#include <array>
#include <string>
#include <cstdint>

using uuid128_t = std::array<uint8_t, 16>;

std::array<char, 37> ToStringArray(const uuid128_t& uuid);
std::string ToString(const uuid128_t& uuid);

bool operator==(const uuid128_t& left, const uuid128_t& right);
bool operator!=(const uuid128_t& left, const uuid128_t& right);

class UuidGenerator
{
public:
        static uuid128_t Generate();
};