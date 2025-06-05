// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVORA_PRIMITIVES_BLOCK_H
#define PRIVORA_PRIMITIVES_BLOCK_H

#include <deque>
#include <type_traits>
#include <boost/foreach.hpp>
#include "primitives/transaction.h"
#include "serialize.h"
#include "uint256.h"
#include "definition.h"
#include "crypto/progpow.h"
#include "privora_params.h"
#include "crypto/progpow.h"

// Can't include sigma.h
namespace sigma {
class CSigmaTxInfo;

} // namespace sigma.

namespace lelantus {
class CLelantusTxInfo;

} // namespace lelantus

namespace spark {
    class CSparkTxInfo;

} // namespace spark

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */

inline int GetZerocoinChainID()
{
    return 0x0001; // We are the first :)
}

class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;   //! std satoshi

    // Privora - ProgPow 
    uint32_t nHeight;
    uint64_t nNonce64;
    uint256 mix_hash;

    static const int CURRENT_VERSION = 2;

    mutable uint256 cachedPoWHash;

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    class CSerializeBlockHeader {};
    class CReadBlockHeader : public CSerActionUnserialize, public CSerializeBlockHeader {};
    class CWriteBlockHeader : public CSerActionSerialize, public CSerializeBlockHeader {};

    template <typename Stream, typename Operation, typename = typename std::enable_if<!std::is_base_of<CSerializeBlockHeader,Operation>::value>::type>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);

        READWRITE(nHeight);
        READWRITE(nNonce64);
        READWRITE(mix_hash);
    }

    template <typename Stream>
    inline void SerializationOp(Stream &s, CReadBlockHeader ser_action) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);

        READWRITE(nHeight);
        READWRITE(nNonce64);
        READWRITE(mix_hash);
    }

    void SetNull()
    {
        nVersion = CBlockHeader::CURRENT_VERSION | (GetZerocoinChainID() * BLOCK_VERSION_CHAIN_START);
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;

        nNonce64 = 0;
        nHeight  = 0;
        mix_hash.SetNull();

        cachedPoWHash.SetNull();
    }

    int GetChainID() const
    {
        return nVersion / BLOCK_VERSION_CHAIN_START;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetPoWHash(int nHeight) const;

    uint256 GetHash() const;
    uint256 GetHashFull(uint256& mix_hash) const;
    
    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
    void InvalidateCachedPoWHash(int nHeight) const;

    std::string ToString() const;

    int GetTargetBlocksSpacing() const;

    CProgPowHeader GetProgPowHeader() const;
    uint256 GetProgPowHeaderHash() const;
    uint256 GetProgPowHashFull(uint256& mix_hash) const;
    uint256 GetProgPowHashLight() const;

};

class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable CTxOut txoutZnode; // znode payment
    mutable std::vector<CTxOut> voutSuperblock; // superblock payment
    mutable bool fChecked;

    // memory only, zerocoin tx info after V3-sigma.
    mutable std::shared_ptr<sigma::CSigmaTxInfo> sigmaTxInfo;

    mutable std::shared_ptr<lelantus::CLelantusTxInfo> lelantusTxInfo;

    mutable std::shared_ptr<spark::CSparkTxInfo> sparkTxInfo;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ~CBlock() {
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
    }

    template <typename Stream>
    inline void SerializationOp(Stream &s, CReadBlockHeader ser_action) {
        CBlockHeader::SerializationOp(s, ser_action);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        txoutZnode = CTxOut();
        voutSuperblock.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;

        block.nHeight    = nHeight;
        block.nNonce64   = nNonce64;
        block.mix_hash   = mix_hash;

        return block;
    }

    std::string ToString() const;

};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

/** Compute the consensus-critical block weight (see BIP 141). */
int64_t GetBlockWeight(const CBlock& tx);

#endif // PRIVORA_PRIMITIVES_BLOCK_H
