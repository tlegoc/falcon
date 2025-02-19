//
// Created by theo on 11/02/2025.
//

#pragma once

#include <array>
#include <string>
#include <cstdint>

using uuid128_t = std::array<uint8_t, 16>;

std::array<char, 37> ToStringArray(const uuid128_t &uuid);

std::string ToString(const uuid128_t &uuid);

bool operator==(const uuid128_t &left, const uuid128_t &right);

bool operator!=(const uuid128_t &left, const uuid128_t &right);

template<>
struct std::hash<uuid128_t> {
    std::size_t operator()(const uuid128_t &uuid) const {
        std::size_t hash1 = 0;
        for (int i = 0; i < 8; i++) {
            hash1 = (hash1 << 4) ^ uuid[i];
        }
        std::size_t hash2 = 0;
        for (int i = 8; i < 16; i++) {
            hash2 = (hash2 << 4) ^ uuid[i];
        }
        return hash1 + hash2;
    }
};

class UuidGenerator {
public:
    static uuid128_t Generate();
};