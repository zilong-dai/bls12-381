#pragma once
#include <cstdlib>
#include <cstdint>
#include <array>
#include <stdexcept>
#include <cstring>
#include <span>

#include "fp.hpp"
#include "g.hpp"

using namespace std;

namespace bls12_381
{
namespace scalar
{

template<size_t N>
void toBytesLE(const array<uint64_t, N>& in, span<uint8_t, N*8> out)
{
    for(uint64_t i = 0; i < N; i++)
    {
        for(uint64_t j = 0, k = 0; j < 8; j++, k += 8)
        {
            out[i*8+j] = static_cast<uint8_t>(in[i] >> k);
        }
    }
}
template<size_t N>
array<uint8_t, N*8> toBytesLE(const array<uint64_t, N>& in)
{
    array<uint8_t, N*8> out;
    toBytesLE(in, out);
    return out;
}

template<size_t N>
void toBytesBE(const array<uint64_t, N>& in, span<uint8_t, N*8> out)
{
    for(uint64_t i = 0; i < N; i++)
    {
        for(uint64_t j = 0, k = 56; j < 8; j++, k -= 8)
        {
            out[i*8+j] = static_cast<uint8_t>(in[N-1-i] >> k);
        }
    }
}
template<size_t N>
array<uint8_t, N*8> toBytesBE(const array<uint64_t, N>& in)
{
    array<uint8_t, N*8> out;
    toBytesBE<N>(in, out);
    return out;
}

template<size_t N>
void fromBytesLE(const span<const uint8_t, N*8>& in, array<uint64_t, N>& out)
{
    for(uint64_t i = 0; i < N; i++)
    {
        uint64_t a = i*8;
        out[i] =
            static_cast<uint64_t>(in[a+0])       | static_cast<uint64_t>(in[a+1]) <<  8 |
            static_cast<uint64_t>(in[a+2]) << 16 | static_cast<uint64_t>(in[a+3]) << 24 |
            static_cast<uint64_t>(in[a+4]) << 32 | static_cast<uint64_t>(in[a+5]) << 40 |
            static_cast<uint64_t>(in[a+6]) << 48 | static_cast<uint64_t>(in[a+7]) << 56;
    }
}
template<size_t N>
array<uint64_t, N> fromBytesLE(const span<const uint8_t, N*8>& in)
{
    array<uint64_t, N> out;
    fromBytesLE(in, out);
    return out;
}

template<size_t N>
void fromBytesBE(const span<const uint8_t, N*8>& in, array<uint64_t, N>& out)
{
    for(uint64_t i = 0; i < N; i++)
    {
        int64_t a = N*8 - i*8;
        out[i] =
            static_cast<uint64_t>(in[a-1])       | static_cast<uint64_t>(in[a-2]) <<  8 |
            static_cast<uint64_t>(in[a-3]) << 16 | static_cast<uint64_t>(in[a-4]) << 24 |
            static_cast<uint64_t>(in[a-5]) << 32 | static_cast<uint64_t>(in[a-6]) << 40 |
            static_cast<uint64_t>(in[a-7]) << 48 | static_cast<uint64_t>(in[a-8]) << 56;
    }
}
template<size_t N>
array<uint64_t, N> fromBytesBE(const span<const uint8_t, N*8>& in)
{
    array<uint64_t, N> out;
    fromBytesBE(in, out);
    return out;
}

// adds two arrays: calculates c = a + b
template<size_t NC, size_t NA, size_t NB>
array<uint64_t, NC> add(const array<uint64_t, NA>& a, const array<uint64_t, NB>& b)
{
    array<uint64_t, NC> c;
    memset(c.data(), 0, NC * sizeof(uint64_t));
    uint64_t carry = 0;
    size_t N = NB > NA ? NB : NA;
    if(N > NC)
    {
        throw invalid_argument("array c not large enough to store result!");
    }
    for(uint64_t i = 0; i < N; i++)
    {
        tie(c[i], carry) = Add64(i >= NA ? 0 : a[i], i >= NB ? 0 : b[i], carry);
    }
    if(carry > 0)
    {
        if(NC <= N+1)
        {
            throw invalid_argument("array c not large enough to store result!");
        }
        c[N+1] = static_cast<uint64_t>(carry);
    }
    return c;
};

// multiplies two arrays: calculates c = a * b
template<size_t NC, size_t NA, size_t NB>
array<uint64_t, NC> mul(const array<uint64_t, NA>& a, const array<uint64_t, NB>& b)
{
    array<uint64_t, NC> c;
    memset(c.data(), 0, NC * sizeof(uint64_t));
    array<uint64_t, NC> buffer;
    uint64_t carry = 0;
    for(uint64_t i = 0; i < NB; i++)
    {
        memset(buffer.data(), 0, NC * sizeof(uint64_t));
        for(uint64_t j = 0; j < NA; j++)
        {
            uint64_t hi, lo;
            tie(hi, lo) = Mul64(a[j], b[i]);
            tie(buffer[i+j], carry) = Add64(buffer[i+j], lo, 0);
            tie(buffer[i+j+1], carry) = Add64(buffer[i+j+1], hi, carry);
        }
        c = add<NC, NC, NC>(c, buffer);
    }
    return c;
};

// compares two arrays: returns -1 if a < b, 0 if a == b and 1 if a > b.
template<size_t N>
int64_t cmp(const array<uint64_t, N>& a, const array<uint64_t, N>& b)
{
    for(int64_t i = N-1; i >= 0; i--)
    {
        if(a[i] < b[i])
        {
            return -1;
        }
        if(a[i] > b[i])
        {
            return 1;
        }
    }
    return 0;
}

// returns the length of the absolute value of s in bits. The bit length of 0 is 0.
template<size_t N>
uint64_t bitLength(const array<uint64_t, N>& s)
{
    for(int64_t i = N-1; i >= 0; i--)
    {
        if(s[i] != 0)
        {
            return (i+1)*64 - __builtin_clzll(s[i]);
        }
    }
    return 0;
}

// shifts an array by a certain amount of bits to the right
template<size_t N>
void rsh(array<uint64_t, N>& arr, uint64_t bits)
{
    uint64_t num_bytes = bits / 64;
    uint64_t num_bits  = bits % 64;

    for(uint64_t i = 0; i < N; i++)
    {
        uint64_t i_from = i + num_bytes;
        uint64_t i_from_p1 = i + num_bytes + 1;
        uint64_t v_from = i_from >= N ? 0 : arr[i_from];
        uint64_t v_from_p1 = i_from_p1 >= N ? 0 : arr[i_from_p1];
         
        arr[i] = v_from >> num_bits | v_from_p1 << (64 - num_bits);
    }
}

} // namespace scalar

void bn_divn_low(uint64_t *c, uint64_t *d, uint64_t *a, int sa, uint64_t *b, int sb);

template<size_t N>
fp fp::modPrime(array<uint64_t, N> k)
{
    array<uint64_t, N> quotient = {0};
    array<uint64_t, N> remainder = {0};
    array<uint64_t, 6> modulus = fp::MODULUS.d;
    bn_divn_low(quotient.data(), remainder.data(), k.data(), N, modulus.data(), 6);
    array<uint64_t, 6> _r = {remainder[0], remainder[1], remainder[2], remainder[3], remainder[4], remainder[5]};
    return fp(_r).toMont();
}

template<size_t N>
fp fp::exp(const array<uint64_t, N>& s) const
{
    fp z = R1;
    uint64_t l = scalar::bitLength(s);
    for(int64_t i = l - 1; i >= 0; i--)
    {
        _mul(&z, &z, &z);
        if((s[i/64] >> (i%64) & 1) == 1)
        {
            _mul(&z, &z, this);
        }
    }
    return z;
}

template<size_t N>
fp2 fp2::exp(const array<uint64_t, N>& s) const
{
    fp2 z = fp2::one();
    uint64_t l = scalar::bitLength(s);
    for(int64_t i = l - 1; i >= 0; i--)
    {
        z = z.square();
        if((s[i/64] >> (i%64) & 1) == 1)
        {
            z = z.mul(*this);
        }
    }
    return z;
}

template<size_t N>
fp6 fp6::exp(const array<uint64_t, N>& s) const
{
    fp6 z = fp6::one();
    uint64_t l = scalar::bitLength(s);
    for(int64_t i = l - 1; i >= 0; i--)
    {
        z = z.square();
        if((s[i/64] >> (i%64) & 1) == 1)
        {
            z = z.mul(*this);
        }
    }
    return z;
}

template<size_t N> fp12 fp12::exp(const array<uint64_t, N>& s) const
{
    fp12 z = fp12::one();
    uint64_t l = scalar::bitLength(s);
    for(int64_t i = l - 1; i >= 0; i--)
    {
        z = z.square();
        if((s[i/64] >> (i%64) & 1) == 1)
        {
            z = z.mul(*this);
        }
    }
    return z;
}

template<size_t N>
fp12 fp12::cyclotomicExp(const array<uint64_t, N>& s) const
{
    fp12 z = fp12::one();
    uint64_t l = scalar::bitLength(s);
    for(int64_t i = l - 1; i >= 0; i--)
    {
        z = z.cyclotomicSquare();
        if((s[i/64] >> (i%64) & 1) == 1)
        {
            z = z.mul(*this);
        }
    }
    return z;
}

template<size_t N>
g1 g1::mulScalar(const array<uint64_t, N>& s) const
{
    g1 q = g1({fp::zero(), fp::zero(), fp::zero()});
    g1 n = *this;
    uint64_t l = scalar::bitLength(s);
    for(uint64_t i = 0; i < l; i++)
    {
        if((s[i/64] >> (i%64) & 1) == 1)
        {
            q = q.add(n);
        }
        n = n.dbl();
    }
    return q;
}

template<size_t N>
g2 g2::mulScalar(const array<uint64_t, N>& s) const
{
    g2 q = g2({fp2::zero(), fp2::zero(), fp2::zero()});
    g2 n = *this;
    uint64_t l = scalar::bitLength(s);
    for(uint64_t i = 0; i < l; i++)
    {
        if((s[i/64] >> (i%64) & 1) == 1)
        {
            q = q.add(n);
        }
        n = n.dbl();
    }
    return q;
}

template<size_t N>
string bytesToHex(const span<const uint8_t, N>& in)
{
    constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    string s(2 + N * 2, ' ');
    s[0] = '0';
    s[1] = 'x';
    for(uint64_t i = 0; i < N; i++)
    {
        s[2 + 2*i]     = hexmap[(in[i] & 0xF0) >> 4];
        s[2 + 2*i+1]   = hexmap[ in[i] & 0x0F      ];
    }
    return s;
}
string bytesToHex(const vector<uint8_t>& in);

template<size_t N>
void hexToBytes(const string& s, span<uint8_t, N> out)
{
    // No checks on the string length in the compile time version!
    uint64_t start_idx = 0;
    if(s[0] == '0' && s[1] == 'x')
    {
        start_idx = 2;
    }

    for(size_t i = 0, j = start_idx; i < N; i++, j += 2)
    {
        out[i] = (s[j] % 32 + 9) % 25 * 16 + (s[j+1] % 32 + 9) % 25;
    }
}
template<size_t N>
array<uint8_t, N> hexToBytes(const string& s)
{
    array<uint8_t, N> out;
    hexToBytes<N>(s, out);
    return out;
}
vector<uint8_t> hexToBytes(const string& s);

} // namespace bls12_381