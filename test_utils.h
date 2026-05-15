#pragma once

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "./identity_k12.h"
#include "./m256.h"

namespace test_utils
{

// Qubic identity: 60 uppercase A–Z characters derived from the 32-byte public key (see qubic-cli key_utils).
inline void getIdentityFromPublicKey(const uint8_t* pubkey, char* dstIdentity, bool isLowerCase = false)
{
    uint8_t publicKey[32];
    std::memcpy(publicKey, pubkey, 32);
    uint16_t identity[61] = {0};
    for (int i = 0; i < 4; i++)
    {
        unsigned long long publicKeyFragment = *reinterpret_cast<unsigned long long*>(&publicKey[i << 3]);
        for (int j = 0; j < 14; j++)
        {
            identity[i * 14 + j] = static_cast<uint16_t>(publicKeyFragment % 26 + (isLowerCase ? 'a' : 'A'));
            publicKeyFragment /= 26;
        }
    }
    unsigned int identityBytesChecksum = 0;
    KangarooTwelve(publicKey, 32, reinterpret_cast<uint8_t*>(&identityBytesChecksum), 3);
    identityBytesChecksum &= 0x3FFFF;
    for (int i = 0; i < 4; i++)
    {
        identity[56 + i] = static_cast<uint16_t>(identityBytesChecksum % 26 + (isLowerCase ? 'a' : 'A'));
        identityBytesChecksum /= 26;
    }
    for (int i = 0; i < 60; i++)
    {
        dstIdentity[i] = static_cast<char>(identity[i]);
    }
    dstIdentity[60] = '\0';
}

static std::string idToIdentity(const m256i& value)
{
    char identity[61] = {0};
    getIdentityFromPublicKey(value.m256i_u8, identity, false);
    return identity;
}


static std::string byteToHex(const unsigned char* byteArray, size_t sizeInByte)
{
    std::ostringstream oss;
    for (size_t i = 0; i < sizeInByte; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]);
    }
    return oss.str();
}

static std::string idToHex(const m256i& value)
{
    return byteToHex(value.m256i_u8, 32);
}

static m256i hexTo32Bytes(const std::string& hex, const int sizeInByte)
{
    if (hex.length() != static_cast<size_t>(sizeInByte) * 2)
    {
        throw std::invalid_argument("Hex string length does not match the expected size");
    }

    m256i byteArray;
    for (int i = 0; i < sizeInByte; ++i)
    {
        byteArray.m256i_u8[i] = static_cast<uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    }

    return byteArray;
}

static void hexToByte(const std::string& hex, const int sizeInByte, unsigned char* out)
{
    if (hex.length() != static_cast<size_t>(sizeInByte) * 2)
    {
        throw std::invalid_argument("Hex string length does not match the expected size");
    }

    for (int i = 0; i < sizeInByte; ++i)
    {
        out[i] = static_cast<unsigned char>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    }
}

static std::vector<std::vector<std::string>> readCSV(const std::string& filename)
{
    std::vector<std::vector<std::string>> data;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> parsedLine;

        while (std::getline(ss, item, ','))
        {
            item.erase(
                std::remove_if(item.begin(), item.end(), [](unsigned char c) { return std::isspace(c) != 0; }),
                item.end());
            parsedLine.push_back(item);
        }
        data.push_back(parsedLine);
    }
    return data;
}

static m256i convertFromString(std::string& rStr)
{
    m256i value{};
    std::stringstream ss(rStr);
    std::string item;
    int i = 0;
    while (std::getline(ss, item, '-') && i < 4)
    {
        value.m256i_u64[i++] = std::stoull(item);
    }
    return value;
}

static std::vector<unsigned long long> convertULLFromString(std::string& rStr)
{
    std::vector<unsigned long long> values;
    std::stringstream ss(rStr);
    std::string item;
    while (std::getline(ss, item, '-'))
    {
        values.push_back(std::stoull(item));
    }
    return values;
}

} // namespace test_utils
