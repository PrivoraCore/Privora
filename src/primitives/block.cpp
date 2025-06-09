// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"
#include "consensus/consensus.h"
#include "validation.h"
#include "mint_spend.h"
#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"
#include "chainparams.h"
#include "crypto/scrypt.h"
#include "crypto/progpow.h"
#include "util.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <string>
#include "precomputed_hash.h"

uint256 CBlockHeader::GetHash() const {
    // return SerializeHash(*this);
    uint256 mix_hash;
        uint256 hash = GetProgPowHashFull(mix_hash);
        const_cast<CBlockHeader*>(this)->mix_hash = mix_hash;
        return hash;
}

uint256 CBlockHeader::GetHashFull(uint256& mix_hash) const {
        uint256 hash = GetProgPowHashFull(mix_hash);
        const_cast<CBlockHeader*>(this)->mix_hash = mix_hash;
        return hash;
}

int CBlockHeader::GetTargetBlocksSpacing() const {
    const Consensus::Params &params = Params().GetConsensus();

    return params.nPowTargetSpacing;
}

CProgPowHeader CBlockHeader::GetProgPowHeader() const {
    return CProgPowHeader {
        nVersion,
        hashPrevBlock,
        hashMerkleRoot,
        nTime,
        nBits,
        nHeight,
        nNonce64,
        mix_hash
    };
}

uint256 CBlockHeader::GetProgPowHeaderHash() const 
{
    return SerializeHash(GetProgPowHeader());
}

uint256 CBlockHeader::GetProgPowHashFull(uint256& mix_hash) const {
    return progpow_hash_full(GetProgPowHeader(), mix_hash);
}

uint256 CBlockHeader::GetProgPowHashLight() const {
    return progpow_hash_light(GetProgPowHeader());
}

uint256 CBlockHeader::GetPoWHash(int nHeight) const {
    if (!cachedPoWHash.IsNull())
        return cachedPoWHash;

    uint256 powHash;
    uint256 mix_hash;
    uint256 hash = GetProgPowHashFull(mix_hash);
    const_cast<CBlockHeader*>(this)->mix_hash = mix_hash;
    
    cachedPoWHash = powHash;
    return powHash;
}

std::string CBlock::ToString() const {
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, mix_hash: %s, nTime=%u, nBits=%08x, nNonce=%u, nNonce64=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        mix_hash.ToString(),
        nTime, nBits, nNonce, nNonce64,
        vtx.size());
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "  " << vtx[i]->ToString() << "\n";
    }
    return s.str();
}

std::string CBlockHeader::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlockHeader("
                   "nVersion=0x%08x, "
                   "hashPrevBlock=%s, "
                   "hashMerkleRoot=%s, "
                   "nTime=%u, "
                   "nBits=%08x, "
                   "nHeight=%d, "
                   "nNonce=%u, "
                   "nNonce64=%llu, "
                   "mix_hash=%s"
                   ")",
                   nVersion,
                   hashPrevBlock.ToString(),
                   hashMerkleRoot.ToString(),
                   nTime,
                   nBits,
                   nHeight,
                   nNonce,
                   static_cast<unsigned long long>(nNonce64),
                   mix_hash.ToString());
    return s.str();
}

int64_t GetBlockWeight(const CBlock& block)
{
//     This implements the weight = (stripped_size * 4) + witness_size formula,
//     using only serialization with and without witness data. As witness_size
//     is equal to total_size - stripped_size, this formula is identical to:
//     weight = (stripped_size * 3) + total_size.
//    return ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * (WITNESS_SCALE_FACTOR - 1) + ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION);
    return ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION);
}