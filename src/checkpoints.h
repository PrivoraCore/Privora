// Copyright (c) 2009-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVORA_CHECKPOINTS_H
#define PRIVORA_CHECKPOINTS_H

#include "uint256.h"

#include <map>

class CBlockIndex;
struct CheckTxData;
struct CCheckpointData;

/**
 * Block-chain checkpoints are compiled-in sanity checks.
 * They are updated every release or three.
 */
namespace Checkpoints
{

//! Returns last CBlockIndex* in mapBlockIndex that is a checkpoint
CBlockIndex* GetLastCheckpoint(const CCheckpointData& data);

//! Return conservative estimate of total number of blocks, 0 if unknown
int GetTotalBlocksEstimate(const CheckTxData& data);

double GuessVerificationProgress(const CheckTxData& data, CBlockIndex* pindex, bool fSigchecks = true);

} //namespace Checkpoints

#endif // PRIVORA_CHECKPOINTS_H
