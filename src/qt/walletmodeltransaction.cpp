// Copyright (c) 2011-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletmodeltransaction.h"

#include "policy/policy.h"
#include "wallet/wallet.h"

WalletModelTransaction::WalletModelTransaction(const QList<SendCoinsRecipient> &_recipients) :
    recipients(_recipients),
    walletTransaction(0),
    keyChange(0),
    fee(0)
{
    walletTransaction = new CWalletTx();
}

WalletModelTransaction::~WalletModelTransaction()
{
    delete keyChange;
    delete walletTransaction;
}

QList<SendCoinsRecipient> WalletModelTransaction::getRecipients()
{
    return recipients;
}

CWalletTx *WalletModelTransaction::getTransaction()
{
    return walletTransaction;
}

unsigned int WalletModelTransaction::getTransactionSize()
{
    return (!walletTransaction ? 0 : ::GetVirtualTransactionSize(*walletTransaction));
}

CAmount WalletModelTransaction::getTransactionFee()
{
    return fee;
}

void WalletModelTransaction::setTransactionFee(const CAmount& newFee)
{
    fee = newFee;
}

void WalletModelTransaction::reassignAmounts(int nChangePosRet)
{
    int i = 0;
    for (QList<SendCoinsRecipient>::iterator it = recipients.begin(); it != recipients.end(); ++it)
    {
        SendCoinsRecipient& rcp = (*it);
        {
            if (i == nChangePosRet)
                i++;
            if (walletTransaction->tx->vout[i].scriptPubKey.IsSparkSMint()) {
                bool ok = true;
                spark::Coin coin(spark::Params::get_default());
                try {
                    spark::ParseSparkMintCoin(walletTransaction->tx->vout[i].scriptPubKey, coin);
                } catch (std::invalid_argument&) {
                    ok = false;
                }

                if (ok) {
                    CSparkMintMeta mintMeta;
                    coin.setSerialContext(spark::getSerialContext(* walletTransaction->tx));
                    if (pwalletMain->sparkWallet->getMintMeta(coin, mintMeta)) {
                        rcp.amount = mintMeta.v;
                    }
                }
            } else {
                rcp.amount = walletTransaction->tx->vout[i].nValue;
            }
            i++;
        }
    }
}

CAmount WalletModelTransaction::getTotalTransactionAmount()
{
    CAmount totalTransactionAmount = 0;
    for (const SendCoinsRecipient &rcp : recipients)
    {
        totalTransactionAmount += rcp.amount;
    }
    return totalTransactionAmount;
}

void WalletModelTransaction::newPossibleKeyChange(CWallet *wallet)
{
    keyChange = new CReserveKey(wallet);
}

CReserveKey *WalletModelTransaction::getPossibleKeyChange()
{
    return keyChange;
}

std::vector<CLelantusEntry>& WalletModelTransaction::getSpendCoins()
{
    return spendCoins;
}

std::vector<CSigmaEntry>& WalletModelTransaction::getSigmaSpendCoins()
{
    return sigmaSpendCoins;
}

std::vector<CHDMint>& WalletModelTransaction::getMintCoins()
{
    return mintCoins;
}