#pragma once

#include <string>
#include <array>
#include <cstdint>

using namespace std;

namespace bls12_381
{

class sha256
{

public:
    sha256();
    void update(const uint8_t * data, size_t length);
    void update(const string &data);
    array<uint8_t, 32> digest();
    void digest(uint8_t* dst);

    static string toString(const array<uint8_t, 32>& digest);

private:
    uint8_t  m_data[64];
    uint32_t m_blocklen;
    uint64_t m_bitlen;
    uint32_t m_state[8]; //A, B, C, D, E, F, G, H

    static array<uint32_t, 64> K;

    static uint32_t rotr(uint32_t x, uint32_t n);
    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g);
    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c);
    static uint32_t sig0(uint32_t x);
    static uint32_t sig1(uint32_t x);
    void transform();
    void pad();
    void revert(array<uint8_t, 32>& hash);
};

} // namespace bls12_381
