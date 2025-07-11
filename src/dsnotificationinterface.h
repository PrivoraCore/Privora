// Copyright (c) 2015 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVORA_DSNOTIFICATIONINTERFACE_H
#define PRIVORA_DSNOTIFICATIONINTERFACE_H

#include "validationinterface.h"

class CDSNotificationInterface : public CValidationInterface
{
public:
    CDSNotificationInterface(CConnman& connmanIn): connman(connmanIn) {}
    virtual ~CDSNotificationInterface() = default;

    // a small helper to initialize current block height in sub-modules on startup
    void InitializeCurrentBlockTip();

protected:
    // CValidationInterface
    void AcceptedBlockHeader(const CBlockIndex *pindexNew) override;
    void NotifyHeaderTip(const CBlockIndex *pindexNew, bool fInitialDownload) override;
    void UpdatedBlockTip(const CBlockIndex *pindexNew, const CBlockIndex *pindexFork, bool fInitialDownload) override;
    void SyncTransaction(const CTransaction &tx, const CBlockIndex *pindex, int posInBlock) override;
    void NotifyMasternodeListChanged(bool undo, const CDeterministicMNList& oldMNList, const CDeterministicMNListDiff& diff) override;
    void NotifyChainLock(const CBlockIndex* pindex) override;

private:
    CConnman& connman;
};

#endif // PRIVORA_DSNOTIFICATIONINTERFACE_H
