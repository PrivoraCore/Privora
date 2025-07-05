// Copyright (c) 2018 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "clientversion.h"
#include "consensus/validation.h"
#include "hash.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "validation.h"

#include "cbtx.h"
#include "deterministicmns.h"
#include "specialtx.h"
#include "spork.h"

#include "llmq/quorums_commitment.h"
#include "llmq/quorums_blockprocessor.h"

bool CheckSpecialTx(const CTransaction& tx, const CBlockIndex* pindexPrev, CValidationState& state)
{
    if (tx.nVersion != 3 || tx.nType == TRANSACTION_NORMAL)
        return true;

    if (pindexPrev && pindexPrev->nHeight + 1 < Params().GetConsensus().DIP0003Height) {
        return state.DoS(10, false, REJECT_INVALID, "bad-tx-type");
    }

    switch (tx.nType) {
    case TRANSACTION_PROVIDER_REGISTER:
        return CheckProRegTx(tx, pindexPrev, state);
    case TRANSACTION_PROVIDER_UPDATE_SERVICE:
        return CheckProUpServTx(tx, pindexPrev, state);
    case TRANSACTION_PROVIDER_UPDATE_REGISTRAR:
        return CheckProUpRegTx(tx, pindexPrev, state);
    case TRANSACTION_PROVIDER_UPDATE_REVOKE:
        return CheckProUpRevTx(tx, pindexPrev, state);
    case TRANSACTION_COINBASE:
        return CheckCbTx(tx, pindexPrev, state);
    case TRANSACTION_QUORUM_COMMITMENT:
        return llmq::CheckLLMQCommitment(tx, pindexPrev, state);
    case TRANSACTION_SPORK:
        return CheckSporkTx(tx, pindexPrev, state);
    case TRANSACTION_SPARK:
        // spark transaction checks are done in other places
        return true;
    case TRANSACTION_LELANTUS:
        // lelantus transaction checks are done in other places
        return true;
    }

    return state.DoS(10, false, REJECT_INVALID, "bad-tx-type-check");
}

bool ProcessSpecialTx(const CTransaction& tx, const CBlockIndex* pindex, CValidationState& state)
{
    if (tx.nVersion != 3 || tx.nType == TRANSACTION_NORMAL) {
        return true;
    }

    switch (tx.nType) {
    case TRANSACTION_PROVIDER_REGISTER:
    case TRANSACTION_PROVIDER_UPDATE_SERVICE:
    case TRANSACTION_PROVIDER_UPDATE_REGISTRAR:
    case TRANSACTION_PROVIDER_UPDATE_REVOKE:
        return true; // handled in batches per block
    case TRANSACTION_COINBASE:
        return true; // nothing to do
    case TRANSACTION_QUORUM_COMMITMENT:
        return true; // handled per block
    case TRANSACTION_SPORK:
        return true;
    case TRANSACTION_LELANTUS:
        return true;
    case TRANSACTION_SPARK:
        return true;
    }

    return state.DoS(100, false, REJECT_INVALID, "bad-tx-type-proc");
}

bool UndoSpecialTx(const CTransaction& tx, const CBlockIndex* pindex)
{
    if (tx.nVersion != 3 || tx.nType == TRANSACTION_NORMAL) {
        return true;
    }

    switch (tx.nType) {
    case TRANSACTION_PROVIDER_REGISTER:
    case TRANSACTION_PROVIDER_UPDATE_SERVICE:
    case TRANSACTION_PROVIDER_UPDATE_REGISTRAR:
    case TRANSACTION_PROVIDER_UPDATE_REVOKE:
        return true; // handled in batches per block
    case TRANSACTION_COINBASE:
        return true; // nothing to do
    case TRANSACTION_QUORUM_COMMITMENT:
        return true; // handled per block
    case TRANSACTION_SPORK:
        return true;
    case TRANSACTION_LELANTUS:
        return true;
    case TRANSACTION_SPARK:
        return true;
    }

    return false;
}

bool ProcessSpecialTxsInBlock(const CBlock& block, const CBlockIndex* pindex, CValidationState& state, bool fJustCheck, bool fCheckCbTxMerleRoots)
{
    static int64_t nTimeLoop = 0;
    static int64_t nTimeQuorum = 0;
    static int64_t nTimeDMN = 0;
    static int64_t nTimeMerkle = 0;

    int64_t nTime1 = GetTimeMicros();

    static const std::set<uint256> blacklistedBlocks = {
        uint256S("00000000001356eb28c2538715d865a7e0f6d62e58bf64b20ef0c54d4f29fa45"),
        uint256S("0000000000ba7b38b93be4b826415a388352ec438fbcf8d2d017eeaea12c2da2"),
        uint256S("0000000000ab2993a2dab51c4321366fa61df81198f51fbcb49710fda6b0c884"),
        uint256S("00000000002d4169448cb0aa1b6708aa70c85f14a97a5029be5039f042cbf103"),
        uint256S("0000000000b611b5a797abe22241eca7db67fe306b88114542cade57565190f9"),
        uint256S("00000000001fc27ced989e7379f9675abea77268a9d349a3cc19b2bed47c68bb"),
        uint256S("0000000000799edd92194ffbab1ffc505144d320a039913d243a579aa1477e95"),
        uint256S("00000000009487216acc680a08cb368c0c9b405bd06a376a1b2431259d55a8c4"),
        uint256S("0000000000450001039aa8037aa30e60c70fe7bcc2cd788fb5a6d21b3823179c"),
        uint256S("0000000000c18ca5411c72738af3a5c108dbadb8cc5f1b8e6b2f645e9af528ae"),
        uint256S("000000000017d310b3638edf488e4532c91209cf2539a6a0f4534953a25cfc6f"),
        uint256S("00000000000b649678aa1e9d10293aa8cf41de36316cf32cef5c5b306defe74f"),
        uint256S("00000000003aac99280f710405ae571063b84125b5493002e399a2e5219f0572"),
        uint256S("0000000000ab50766c1e320b47c7a77b65a171e92cfc3198b2af9af884b2f24a"),
        uint256S("0000000001b650f5348bc901f8eccd64270f9197aaf3d7174b44414cb7ba3764"),
        uint256S("00000000004f8730de4ca7275a7949a3647be1e34555e74a06d61f2cb481f4dd"),
        uint256S("00000000004bb74bd9630fe7eed52f71e0a9f1cb1a1dfc7e1be0d91e144e595e")
    };

    if (blacklistedBlocks.count(pindex->GetBlockHash())) {
        LogPrintf("Skipping processing of blacklisted block %s at height %d\n", pindex->GetBlockHash().ToString(), pindex->nHeight);
        return true;
    }

    for (int i = 0; i < (int)block.vtx.size(); i++) {
        const CTransaction& tx = *block.vtx[i];
        if (!CheckSpecialTx(tx, pindex->pprev, state)) {
            return false;
        }
        if (!ProcessSpecialTx(tx, pindex, state)) {
            return false;
        }
    }

    int64_t nTime2 = GetTimeMicros(); nTimeLoop += nTime2 - nTime1;
    LogPrint("bench", "        - Loop: %.2fms [%.2fs]\n", 0.001 * (nTime2 - nTime1), nTimeLoop * 0.000001);

    if (!llmq::quorumBlockProcessor->ProcessBlock(block, pindex, state)) {
        return false;
    }

    int64_t nTime3 = GetTimeMicros(); nTimeQuorum += nTime3 - nTime2;
    LogPrint("bench", "        - quorumBlockProcessor: %.2fms [%.2fs]\n", 0.001 * (nTime3 - nTime2), nTimeQuorum * 0.000001);

    if (!deterministicMNManager->ProcessBlock(block, pindex, state, fJustCheck)) {
        return false;
    }

    int64_t nTime4 = GetTimeMicros(); nTimeDMN += nTime4 - nTime3;
    LogPrint("bench", "        - deterministicMNManager: %.2fms [%.2fs]\n", 0.001 * (nTime4 - nTime3), nTimeDMN * 0.000001);

    if (fCheckCbTxMerleRoots && !CheckCbTxMerkleRoots(block, pindex, state)) {
        return false;
    }

    int64_t nTime5 = GetTimeMicros(); nTimeMerkle += nTime5 - nTime4;
    LogPrint("bench", "        - CheckCbTxMerkleRoots: %.2fms [%.2fs]\n", 0.001 * (nTime5 - nTime4), nTimeMerkle * 0.000001);

    return true;
}

bool UndoSpecialTxsInBlock(const CBlock& block, const CBlockIndex* pindex)
{
    for (int i = (int)block.vtx.size() - 1; i >= 0; --i) {
        const CTransaction& tx = *block.vtx[i];
        if (!UndoSpecialTx(tx, pindex)) {
            return false;
        }
    }

    if (!deterministicMNManager->UndoBlock(block, pindex)) {
        return false;
    }

    if (!llmq::quorumBlockProcessor->UndoBlock(block, pindex)) {
        return false;
    }

    return true;
}

uint256 CalcTxInputsHash(const CTransaction& tx)
{
    CHashWriter hw(CLIENT_VERSION, SER_GETHASH);
    for (const auto& in : tx.vin) {
        hw << in.prevout;
    }
    return hw.GetHash();
}
