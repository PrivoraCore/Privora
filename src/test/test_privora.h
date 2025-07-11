// Copyright (c) 2015-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVORA_TEST_TEST_PRIVORA_H
#define PRIVORA_TEST_TEST_PRIVORA_H

#include "chainparamsbase.h"
#include "key.h"
#include "pubkey.h"
#include "txdb.h"
#include "txmempool.h"
#include "evo/evodb.h"
#include "evo/deterministicmns.h"

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

/** Basic testing setup.
 * This just configures logging and chain parameters.
 */
struct BasicTestingSetup {
    ECCVerifyHandle globalVerifyHandle;

    BasicTestingSetup(const std::string& chainName = CBaseChainParams::MAIN);
    ~BasicTestingSetup();
};

/** Testing setup that configures a complete environment.
 * Included are data directory, coins database, script check threads setup.
 */
class CConnman;
struct TestingSetup: public BasicTestingSetup {
    CCoinsViewDB *pcoinsdbview;
    boost::filesystem::path pathTemp;
    boost::thread_group threadGroup;
    CConnman* connman;

    TestingSetup(const std::string& chainName = CBaseChainParams::MAIN, std::string suf = "");
    ~TestingSetup();
};

class CBlock;
struct CMutableTransaction;
class CScript;

//
// Testing fixture that pre-creates a
// 100-block REGTEST-mode block chain
//
struct TestChain100Setup : public TestingSetup {
    TestChain100Setup(int nBlocks = 100);

    CBlock CreateBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey);
    CBlock CreateBlock(const std::vector<CMutableTransaction>& txns, const CKey& scriptKey);
    // Create a new block with just given transactions, coinbase paying to
    // scriptPubKey, and try to add it to the current chain.
    CBlock CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns,
                                 const CScript& scriptPubKey, bool * processBlockResult = nullptr);

    CBlock CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns,
                                 const CKey& scriptKey, bool * processBlockResult = nullptr);

    ~TestChain100Setup();

    std::vector<CTransaction> coinbaseTxns; // For convenience, coinbase transactions
    CKey coinbaseKey; // private/public key needed to spend coinbase transactions

    bool fAllowMempoolTxsInCreateBlock{false}; // do not remove transactions from mempool during CreateBlock processing
};

struct TestChainDIP3Setup : public TestChain100Setup {
    TestChainDIP3Setup() : TestChain100Setup(850) {}
};

struct TestChainDIP3BeforeActivationSetup : public TestChain100Setup {
    TestChainDIP3BeforeActivationSetup() : TestChain100Setup(498) {}
};

class CTxMemPoolEntry;
class CTxMemPool;

struct TestMemPoolEntryHelper
{
    // Default values
    CAmount nFee;
    int64_t nTime;
    double dPriority;
    unsigned int nHeight;
    bool spendsCoinbase;
    unsigned int sigOpCost;
    LockPoints lp;

    TestMemPoolEntryHelper() :
        nFee(0), nTime(0), dPriority(0.0), nHeight(1),
        spendsCoinbase(false), sigOpCost(4) { }

    CTxMemPoolEntry FromTx(const CMutableTransaction &tx, CTxMemPool *pool = NULL);
    CTxMemPoolEntry FromTx(const CTransaction &tx, CTxMemPool *pool = NULL);

    // Change the default value
    TestMemPoolEntryHelper &Fee(CAmount _fee) { nFee = _fee; return *this; }
    TestMemPoolEntryHelper &Time(int64_t _time) { nTime = _time; return *this; }
    TestMemPoolEntryHelper &Priority(double _priority) { dPriority = _priority; return *this; }
    TestMemPoolEntryHelper &Height(unsigned int _height) { nHeight = _height; return *this; }
    TestMemPoolEntryHelper &SpendsCoinbase(bool _flag) { spendsCoinbase = _flag; return *this; }
    TestMemPoolEntryHelper &SigOpsCost(unsigned int _sigopsCost) { sigOpCost = _sigopsCost; return *this; }
};

std::string privora_address_to_privora(const std::string address);
size_t FindZnodeOutput(CTransaction const & tx);
#endif
