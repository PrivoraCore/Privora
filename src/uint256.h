// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVORA_UINT256_H
#define PRIVORA_UINT256_H

#include <assert.h>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include "crypto/common.h"

/** Template base class for fixed-sized opaque blobs. */
template<unsigned int BITS>
class base_blob
{
protected:
    enum { WIDTH=BITS/8 };
    uint8_t data[WIDTH];
public:
    base_blob()
    {
        memset(data, 0, sizeof(data));
    }

    explicit base_blob(const std::vector<unsigned char>& vch);
    explicit base_blob(const std::array<unsigned char, WIDTH>& vch);

    bool IsNull() const
    {
        for (int i = 0; i < WIDTH; i++)
            if (data[i] != 0)
                return false;
        return true;
    }

    void SetNull()
    {
        memset(data, 0, sizeof(data));
    }

    inline int Compare(const base_blob& other) const { return memcmp(data, other.data, sizeof(data)); }

    friend inline bool operator==(const base_blob& a, const base_blob& b) { return a.Compare(b) == 0; }
    friend inline bool operator!=(const base_blob& a, const base_blob& b) { return a.Compare(b) != 0; }
    friend inline bool operator<(const base_blob& a, const base_blob& b) { return a.Compare(b) < 0; }

    std::string GetHex() const;
    void SetHex(const char* psz);
    void SetHex(const std::string& str);
    std::string ToString() const;
    base_blob<BITS> uintS(const char *str) const;
    base_blob<BITS> uintS(const std::string& str) const;

    unsigned char* begin()
    {
        return &data[0];
    }

    unsigned char* end()
    {
        return &data[WIDTH];
    }

    const unsigned char* begin() const
    {
        return &data[0];
    }

    const unsigned char* end() const
    {
        return &data[WIDTH];
    }

    unsigned int size() const
    {
        return sizeof(data);
    }

    uint64_t GetUint64(int pos) const
    {
        const uint8_t* ptr = data + pos * 8;
        return ((uint64_t)ptr[0]) | \
               ((uint64_t)ptr[1]) << 8 | \
               ((uint64_t)ptr[2]) << 16 | \
               ((uint64_t)ptr[3]) << 24 | \
               ((uint64_t)ptr[4]) << 32 | \
               ((uint64_t)ptr[5]) << 40 | \
               ((uint64_t)ptr[6]) << 48 | \
               ((uint64_t)ptr[7]) << 56;
    }

    uint32_t GetFirstUint32() const
    {
        const uint8_t* ptr = data;
        return ((uint32_t)ptr[0]) | \
               ((uint32_t)ptr[1]) << 8 | \
               ((uint32_t)ptr[2]) << 16 | \
               ((uint32_t)ptr[3]) << 24;
    }

    template<typename Stream>
    void Serialize(Stream& s) const
    {
        s.write((char*)data, sizeof(data));
    }

    template<typename Stream>
    void Unserialize(Stream& s)
    {
        s.read((char*)data, sizeof(data));
    }
};

/** 160-bit opaque blob.
 * @note This type is called uint160 for historical reasons only. It is an opaque
 * blob of 160 bits and has no integer operations.
 */
class uint160 : public base_blob<160> {
public:
    uint160() {}
    uint160(const base_blob<160>& b) : base_blob<160>(b) {}
    explicit uint160(const std::vector<unsigned char>& vch) : base_blob<160>(vch) {}
};

/** 256-bit opaque blob.
 * @note This type is called uint256 for historical reasons only. It is an
 * opaque blob of 256 bits and has no integer operations. Use arith_uint256 if
 * those are required.
 */
class uint256 : public base_blob<256> {
public:
    uint256() {}
    uint256(const base_blob<256>& b) : base_blob<256>(b) {}
    explicit uint256(const std::vector<unsigned char>& vch) : base_blob<256>(vch) {}

    /** A cheap hash function that just returns 64 bits from the result, it can be
     * used when the contents are considered uniformly random. It is not appropriate
     * when the value can easily be influenced from outside as e.g. a network adversary could
     * provide values to trigger worst-case behavior.
     */
    uint64_t GetCheapHash() const
    {
        return ReadLE64(data);
    }
};

/* uint256 from const char *.
 * This is a separate function because the constructor uint256(const char*) can result
 * in dangerously catching uint256(0).
 */
inline uint256 uint256S(const char *str)
{
    uint256 rv;
    rv.SetHex(str);
    return rv;
}
/* uint256 from std::string.
 * This is a separate function because the constructor uint256(const std::string &str) can result
 * in dangerously catching uint256(0) via std::string(const char*).
 */
inline uint256 uint256S(const std::string& str)
{
    uint256 rv;
    rv.SetHex(str);
    return rv;
}

/** 512-bit opaque blob.
 * @note This type is called uint512 for historical reasons only. It is an
 * opaque blob of 512 bits and has no integer operations. Use arith_uint512 if
 * those are required.
 */
class uint512 : public base_blob<512> {
public:
    uint512() {}
    uint512(const base_blob<512>& b) : base_blob<512>(b) {}
    explicit uint512(const std::vector<unsigned char>& vch) : base_blob<512>(vch) {}
    explicit uint512(const std::array<unsigned char, 64>& vch) : base_blob<512>(vch) {}

     /** A cheap hash function that just returns 64 bits from the result, it can be
     * used when the contents are considered uniformly random. It is not appropriate
     * when the value can easily be influenced from outside as e.g. a network adversary could
     * provide values to trigger worst-case behavior.
     */
    uint64_t GetCheapHash() const
    {
        return ReadLE64(data);
    }

     uint256 trim256() const
    {
        uint256 ret;
        memcpy(ret.begin(), (*this).begin(), ret.size());
        return ret;
    }
};

namespace std {

template<unsigned Size>
struct hash<base_blob<Size>>
{
    size_t operator()(const base_blob<Size>& b) const
    {
        return hash<string>()(string(b.begin(), b.end()));
    }
};

template<>
struct hash<uint256> : hash<base_blob<256>>
{
};

template<>
struct hash<uint160> : hash<base_blob<160>>
{
};

} // namespace std

#endif // PRIVORA_UINT256_H
