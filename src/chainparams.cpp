// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"
#include "consensus/consensus.h"
#include "privora_params.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "privora_bignum/bignum.h"
#include "blacklists.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"
#include "arith_uint256.h"

using namespace secp_primitives;

static CBlock CreateGenesisBlock(const char *pszTimestamp, const CScript &genesisOutputScript, const CScript &developmentOutputScript, uint32_t nTime, uint64_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount &genesisReward, const CAmount &developmentReward, std::vector<unsigned char> extraNonce) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(2);
    txNew.vin[0].scriptSig = CScript() << std::vector<unsigned char>(pszTimestamp, pszTimestamp + strlen(pszTimestamp)) << extraNonce;
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;
    txNew.vout[1].nValue = developmentReward;
    txNew.vout[1].scriptPubKey = developmentOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce64 = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint64_t nNonce, uint32_t nBits, int32_t nVersion, const std::vector<uint8_t> &genesisOutputScriptHex, const std::vector<uint8_t> &developmentOutputScriptHex, const CAmount &genesisReward, const CAmount &developmentReward, std::vector<unsigned char> extraNonce) {
    const char *pszTimestamp = "CNN - 08/06/2025 - Trump deploys National Guard to Los Angeles amid immigration protests.";

    const CScript genesisOutputScript = CScript(genesisOutputScriptHex.begin(), genesisOutputScriptHex.end());
    const CScript developmentOutputScript = CScript(developmentOutputScriptHex.begin(), developmentOutputScriptHex.end());

    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, developmentOutputScript, nTime, nNonce, nBits, nVersion, genesisReward, developmentReward, extraNonce);
}

// this one is for testing only
static Consensus::LLMQParams llmq5_60 = {
        .type = Consensus::LLMQ_5_60,
        .name = "llmq_5_60",
        .size = 5,
        .minSize = 3,
        .threshold = 3,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 8,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3,
};

// to use on testnet
static Consensus::LLMQParams llmq10_70 = {
        .type = Consensus::LLMQ_10_70,
        .name = "llmq_10_70",
        .size = 10,
        .minSize = 8,
        .threshold = 7,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 8,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3,
};

static Consensus::LLMQParams llmq50_60 = {
        .type = Consensus::LLMQ_50_60,
        .name = "llmq_50_60",
        .size = 50,
        .minSize = 40,
        .threshold = 30,

        .dkgInterval = 18, // one DKG per 90 minutes
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 16,
        .dkgBadVotesThreshold = 40,

        .signingActiveQuorumCount = 16, // a full day worth of LLMQs

        .keepOldConnections = 17,
};

static Consensus::LLMQParams llmq400_60 = {
        .type = Consensus::LLMQ_400_60,
        .name = "llmq_400_60",
        .size = 400,
        .minSize = 300,
        .threshold = 240,

        .dkgInterval = 12 * 12, // one DKG every 12 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 28,
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // two days worth of LLMQs

        .keepOldConnections = 5,
};

// Used for deployment and min-proto-version signalling, so it needs a higher threshold
static Consensus::LLMQParams llmq400_85 = {
        .type = Consensus::LLMQ_400_85,
        .name = "llmq_400_85",
        .size = 400,
        .minSize = 350,
        .threshold = 340,

        .dkgInterval = 12 * 24, // one DKG every 24 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 48, // give it a larger mining window to make sure it is mined
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // two days worth of LLMQs

        .keepOldConnections = 5,
};

static std::array<int,21> standardSparkNamesFee = {
    -1,
    1000,
    100,
    10, 10, 10,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";

        consensus.chainType = Consensus::chainMain;


        consensus.nSubsidyHalvingInterval = 420768;

        consensus.nMasternodePayout = 40;
        consensus.nDevelopmentFundPercent = 10;

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x19,0xe8,0xa2,0xd1,0xa2,0x31,0x16,0x5d,0x45,0x0f,0x1c,0x3d,0x26,0x0d,0x9f,0xfc,0x0d,0x7c,0x57,0xa4,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0x7a,0x73,0x87,0x85,0x95,0x71,0xd2,0xff,0xca,0x8a,0x7e,0xf7,0xef,0x15,0x3a,0xce,0x45,0x61,0xf9,0xb8,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = INT_MAX;
        consensus.BIP66Height = INT_MAX;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 250;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1475020800; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1479168000; // November 15th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1510704000; // November 15th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0 "); //184200

        consensus.nMasternodePaymentsStartBlock = 10000;

        // evo znodes
        consensus.DIP0003Height = 100; // Approximately June 22 2020, 12:00 UTC
        consensus.DIP0003EnforcementHeight = 110; // Approximately July 13 2020, 12:00 UTC
        consensus.DIP0008Height = 120; // Approximately Jan 28 2021, 11:00 UTC
        consensus.nEvoZnodeMinimumConfirmations = 15;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.nLLMQPowTargetSpacing = 1*60;
        consensus.llmqChainLocks = Consensus::LLMQ_400_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_50_60;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 24;
        consensus.nInstantSendBlockFilteringStartHeight = 421150;   // Approx Nov 2 2021 06:00:00 GMT+0000

        consensus.nDisableZerocoinStartBlock = 0;

        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in privora

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
       `  * a large 32-bit integer with any alignment.
         */
        //btzc: update privora pchMessage
        pchMessageStart[0] = 0xe3;
        pchMessageStart[1] = 0xd9;
        pchMessageStart[2] = 0xfe;
        pchMessageStart[3] = 0xf1;
        nDefaultPort = 8168;
        nPruneAfterHeight = 100000;

        std::vector<unsigned char> extraNonce(4);
        extraNonce[0] = 0x34;
        extraNonce[1] = 0xab;
        extraNonce[2] = 0x6c;
        extraNonce[3] = 0xfe;
        genesis = CreateGenesisBlock(1749384000, 2330, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);

        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);
        assert(consensus.hashGenesisBlock == uint256S("0x0007229e4654a3037b5d2a9642a846fdb6a67f519702768d3dd3a678d2dcb20a"));
        assert(genesis.hashMerkleRoot == uint256S("0x6b4e4f4568408907285bb00dced1103aa1879fdc61bb27b28c20534845e9bbb0"));
        assert(genesis.mix_hash == uint256S("0x3a70e04064c32aeb425812cb9ecbb94af3bd822d44f6d7b3416ae2a9ebc4cfbd"));

        vSeeds.push_back(CDNSSeedData("dns-mainnet-1.privora.org", "dns-mainnet-1.privora.org", false));
        vSeeds.push_back(CDNSSeedData("dns-mainnet-2.privora.org", "dns-mainnet-2.privora.org", false));

        // Note that of those with the service bits flag, most only support a subset of possible options
        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 82);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 7);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0xbb};   // EXX prefix for the address
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 210);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container < std::vector < unsigned char > > ();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fAllowMultiplePorts = false;

        checkpointData = (CCheckpointData) {
                boost::assign::map_list_of
                (0, uint256S("0x0007229e4654a3037b5d2a9642a846fdb6a67f519702768d3dd3a678d2dcb20a"))
        };

        chainTxData = ChainTxData{
                0, // * UNIX timestamp of last checkpoint block
                0,     // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
                0      // * estimated number of transactions per second after checkpoint
        };

        // Sigma related values.
        consensus.nSigmaStartBlock = INT_MAX;
        consensus.nSigmaPaddingBlock = INT_MAX;
        consensus.nDisableUnpaddedSigmaBlock = INT_MAX;
        consensus.nStartSigmaBlacklist = INT_MAX;
        consensus.nRestartSigmaWithBlacklistCheck = INT_MAX;
        consensus.nOldSigmaBanBlock = INT_MAX;
        consensus.nLelantusStartBlock = INT_MAX;
        consensus.nLelantusFixesStartBlock = INT_MAX;
        consensus.nLelantusGracefulPeriod = INT_MAX;
        consensus.nSigmaEndBlock = INT_MAX;
        consensus.nZerocoinV2MintMempoolGracefulPeriod = INT_MAX;
        consensus.nZerocoinV2MintGracefulPeriod = INT_MAX;
        consensus.nZerocoinV2SpendMempoolGracefulPeriod = INT_MAX;
        consensus.nZerocoinV2SpendGracefulPeriod = INT_MAX;
        consensus.nMaxSigmaInputPerBlock = INT_MAX;
        consensus.nMaxValueSigmaSpendPerBlock = INT_MAX;
        consensus.nMaxSigmaInputPerTransaction = INT_MAX;
        consensus.nMaxValueSigmaSpendPerTransaction = INT_MAX;
        consensus.nMaxLelantusInputPerBlock = INT_MAX;
        consensus.nMaxValueLelantusSpendPerBlock = INT_MAX;
        consensus.nMaxLelantusInputPerTransaction = INT_MAX;
        consensus.nMaxValueLelantusSpendPerTransaction = INT_MAX;
        consensus.nMaxValueLelantusMint = INT_MAX;

        consensus.nZerocoinToSigmaRemintWindowSize = 0;

        for (const auto& str : lelantus::lelantus_blacklist) {
            GroupElement coin;
            try {
                coin.deserialize(ParseHex(str).data());
            } catch (const std::exception &) {
                continue;
            }
            consensus.lelantusBlacklist.insert(coin);
        }

        for (const auto& str : sigma::sigma_blacklist) {
            GroupElement coin;
            try {
                coin.deserialize(ParseHex(str).data());
            } catch (const std::exception &) {
                continue;
            }
            consensus.sigmaBlacklist.insert(coin);
        }

        consensus.evoSporkKeyID = "a78fERshquPsTv2TuKMSsxTeKom56uBwLP";
        consensus.nEvoSporkStartBlock = 1;
        consensus.nEvoSporkStopBlock = 0;
        consensus.nEvoSporkStopBlockExtensionVersion = 0;
        consensus.nEvoSporkStopBlockPrevious = 1 + 1*24*12*365; // one year after lelantus
        consensus.nEvoSporkStopBlockExtensionGracefulPeriod = 24*12*14; // two weeks

        // reorg
        consensus.nMaxReorgDepth = 5;
        consensus.nMaxReorgDepthEnforcementBlock = 338000;

        // whitelist
        consensus.txidWhitelist.insert(uint256S(""));

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = DANDELION_EMBARGO_MINIMUM;
        consensus.nDandelionEmbargoAvgAdd = DANDELION_EMBARGO_AVG_ADD;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 222400;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = 401580;

        // exchange address
        consensus.nExchangeAddressStartBlock = consensus.nSparkStartBlock;

        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nSparkStartBlock = 100;

        // spark names
        consensus.nSparkNamesStartBlock = 105;  // ~ May 28th 2025
        consensus.nSparkNamesFee = standardSparkNamesFee;
    }
    virtual bool SkipUndoForBlock(int nHeight) const
    {
        return nHeight == 293526;
    }
    virtual bool ApplyUndoForTxout(int nHeight, uint256 const & txid, int n) const
    {
        // We only apply first 23 tx inputs UNDOs for the tx 7702 in block 293526
        if (!SkipUndoForBlock(nHeight)) {
            return true;
        }
        static std::map<uint256, int> const txs = { {uint256S(""), 22} };
        std::map<uint256, int>::const_iterator const itx = txs.find(txid);
        if (itx == txs.end()) {
            return false;
        }
        if (n <= itx->second) {
            return true;
        }
        return false;
    }
};

static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.chainType = Consensus::chainTestnet;

        consensus.nSubsidyHalvingInterval = 420768;

        consensus.nMasternodePayout = 40;
        consensus.nDevelopmentFundPercent = 10;

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x1f,0x8c,0x31,0xd1,0xa9,0xa7,0xfa,0xf5,0x85,0x33,0xf2,0xb9,0x20,0x37,0x48,0x71,0x14,0x91,0xa9,0xc2,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0x07,0x91,0xec,0x8e,0x66,0x4f,0xe3,0x28,0xc9,0x19,0x01,0xcf,0xc9,0xd6,0xdb,0xbe,0x02,0x61,0xaa,0xfa,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = INT_MAX;
        consensus.BIP66Height = INT_MAX;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 30;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S(" "); // 50000

        consensus.nMasternodePaymentsStartBlock = 10000;

        nMaxTipAge = 0x7fffffff; // allow mining on top of old blocks for testnet

        // evo znodes
        consensus.DIP0003Height = 3340;
        consensus.DIP0003EnforcementHeight = 3800;

        consensus.DIP0008Height = 25000;
        consensus.nEvoZnodeMinimumConfirmations = 0;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_10_70] = llmq10_70;
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.nLLMQPowTargetSpacing = 20;
        consensus.llmqChainLocks = Consensus::LLMQ_10_70;
        consensus.llmqForInstantSend = Consensus::LLMQ_10_70;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;
        consensus.nInstantSendBlockFilteringStartHeight = 48136;

        consensus.nDisableZerocoinStartBlock = 0;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        pchMessageStart[0] = 0xcf;
        pchMessageStart[1] = 0xfc;
        pchMessageStart[2] = 0xbe;
        pchMessageStart[3] = 0xea;
        nDefaultPort = 18168;
        nPruneAfterHeight = 1000;
        /**
         * btzc: testnet params
         * nTime: 1414776313
         * nNonce: 1620571
         */
        std::vector<unsigned char> extraNonce(4);
        extraNonce[0] = 0xac;
        extraNonce[1] = 0xb3;
        extraNonce[2] = 0x21;
        extraNonce[3] = 0x64;

        genesis = CreateGenesisBlock(1749384000, 843, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x000345d9426a940aca152a26c3eb00ede55165dd0d34ab5766fddf6b48293bc6"));
        assert(genesis.hashMerkleRoot == uint256S("0x86419aef7c300a8757185d77e0e183bed93ff7c0257ddbced1e23cebd4d0e9bb"));
        assert(genesis.mix_hash == uint256S("0xd9da891720e8d94fe7d2e9d257b2cfd7b46ae8a9c918672d358900241db8fd1b"));

        vFixedSeeds.clear();
        vSeeds.clear();

        vSeeds.push_back(CDNSSeedData("EVO1", "evo1.privora.org", false));
        vSeeds.push_back(CDNSSeedData("EVO2", "evo2.privora.org", false));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 65);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 178);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0xb1};   // EXT prefix for the address
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 185);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container < std::vector < unsigned char > > ();
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fAllowMultiplePorts = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, uint256S("0x"))
        };

        chainTxData = ChainTxData{
            1414776313,
            0,
            0.001
        };

        // Sigma related values.
        consensus.nSigmaStartBlock = 1;
        consensus.nSigmaPaddingBlock = 1;
        consensus.nDisableUnpaddedSigmaBlock = 1;
        consensus.nStartSigmaBlacklist = INT_MAX;
        consensus.nRestartSigmaWithBlacklistCheck = INT_MAX;
        consensus.nOldSigmaBanBlock = 1;

        consensus.nLelantusStartBlock = ZC_LELANTUS_TESTNET_STARTING_BLOCK;
        consensus.nLelantusFixesStartBlock = ZC_LELANTUS_TESTNET_FIXES_START_BLOCK;

        consensus.nSparkStartBlock = SPARK_TESTNET_START_BLOCK;
        consensus.nLelantusGracefulPeriod = LELANTUS_TESTNET_GRACEFUL_PERIOD;
        consensus.nSigmaEndBlock = ZC_SIGMA_TESTNET_END_BLOCK;
        consensus.nZerocoinV2MintMempoolGracefulPeriod = ZC_V2_MINT_TESTNET_GRACEFUL_MEMPOOL_PERIOD;
        consensus.nZerocoinV2MintGracefulPeriod = ZC_V2_MINT_TESTNET_GRACEFUL_PERIOD;
        consensus.nZerocoinV2SpendMempoolGracefulPeriod = ZC_V2_SPEND_TESTNET_GRACEFUL_MEMPOOL_PERIOD;
        consensus.nZerocoinV2SpendGracefulPeriod = ZC_V2_SPEND_TESTNET_GRACEFUL_PERIOD;
        consensus.nMaxSigmaInputPerBlock = ZC_SIGMA_INPUT_LIMIT_PER_BLOCK;
        consensus.nMaxValueSigmaSpendPerBlock = ZC_SIGMA_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSigmaInputPerTransaction = ZC_SIGMA_INPUT_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSigmaSpendPerTransaction = ZC_SIGMA_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxLelantusInputPerBlock = ZC_LELANTUS_INPUT_LIMIT_PER_BLOCK;
        consensus.nMaxValueLelantusSpendPerBlock = 1100 * COIN;
        consensus.nMaxLelantusInputPerTransaction = ZC_LELANTUS_INPUT_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueLelantusSpendPerTransaction = 1001 * COIN;
        consensus.nMaxValueLelantusMint = 1001 * COIN;
        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nZerocoinToSigmaRemintWindowSize = 0;

        for (const auto& str : lelantus::lelantus_testnet_blacklist) {
            GroupElement coin;
            try {
                coin.deserialize(ParseHex(str).data());
            } catch (const std::exception &) {
                continue;
            }
            consensus.lelantusBlacklist.insert(coin);
        }

        consensus.evoSporkKeyID = "TWSEa1UsZzDHywDG6CZFDNdeJU6LzhbbBL";
        consensus.nEvoSporkStartBlock = 22000;
        consensus.nEvoSporkStopBlock = 40000;
        consensus.nEvoSporkStopBlockExtensionVersion = 0;

        // reorg
        consensus.nMaxReorgDepth = 4;
        consensus.nMaxReorgDepthEnforcementBlock = 25150;

        // whitelist
        consensus.txidWhitelist.insert(uint256S("44b3829117bd248544c71b430d585cb88b4ce156a7d4fdb9ef3ae96efa8f09d3"));

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = DANDELION_TESTNET_EMBARGO_MINIMUM;
        consensus.nDandelionEmbargoAvgAdd = DANDELION_TESTNET_EMBARGO_AVG_ADD;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 1;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = 35000;

        // exchange address
        consensus.nExchangeAddressStartBlock = 147000;

        // spark names
        consensus.nSparkNamesStartBlock = 174000;
        consensus.nSparkNamesFee = standardSparkNamesFee;
    }
};

static CTestNetParams testNetParams;

/**
 * Devnet (testnet for experimental stuff)
 */
class CDevNetParams : public CChainParams {
public:
    CDevNetParams() {
        strNetworkID = "dev";

        consensus.chainType = Consensus::chainDevnet;


        consensus.nSubsidyHalvingInterval = 420768;

        consensus.nMasternodePayout = 40;
        consensus.nDevelopmentFundPercent = 10;

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x24,0x2c,0xe,0x36,0x94,0xb6,0x4e,0x3d,0x21,0x44,0x01,0x6f,0xe3,0xb3,0x95,0xa6,0x2d,0xcd,0x66,0x4b,0xf8,0x8,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0xb1,0xa0,0x35,0xe1,0x68,0x52,0xe3,0x6b,0xf8,0xb7,0x30,0x39,0x5d,0xbd,0x45,0xf0,0xb7,0xa5,0x1f,0x27,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = INT_MAX;
        consensus.BIP66Height = INT_MAX;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 30;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S(" "); // 50000

        // Znode params testnet
        consensus.nMasternodePaymentsStartBlock = 10000;
        nMaxTipAge = 0x7fffffff; // allow mining on top of old blocks for testnet

        // evo znodes
        consensus.DIP0003Height = 800;
        consensus.DIP0003EnforcementHeight = 820;

        consensus.DIP0008Height = 850;
        consensus.nEvoZnodeMinimumConfirmations = 0;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_5_60] = llmq5_60;
        consensus.llmqs[Consensus::LLMQ_10_70] = llmq10_70;
        consensus.nLLMQPowTargetSpacing = 20;
        consensus.llmqChainLocks = Consensus::LLMQ_5_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_5_60;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;
        consensus.nInstantSendBlockFilteringStartHeight = 1000;

        consensus.nDisableZerocoinStartBlock = 0;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        pchMessageStart[0] = 0xcf;
        pchMessageStart[1] = 0xfc;
        pchMessageStart[2] = 0xbe;
        pchMessageStart[3] = 0xeb;

        nDefaultPort = 38168;
        nPruneAfterHeight = 1000;

        std::vector<unsigned char> extraNonce(4);
        extraNonce[0] = 0x0f;
        extraNonce[1] = 0xd4;
        extraNonce[2] = 0x7c;
        extraNonce[3] = 0x35;

        genesis = CreateGenesisBlock(1749384000, 100, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x0007b7d63bfa4d68633b66b7c8f35aff2520a925c502821c821fddb9ecf16d7e"));
        assert(genesis.hashMerkleRoot == uint256S("0x0a0eef29fa17cd3358bf81bd1198ca8d87acb74c2fb6b9ddb608300c50dba79e"));
        assert(genesis.mix_hash == uint256S("0xac84b070724809403c015438761e5faeff582a08defd9bcb09df01b9049e5873"));


        vFixedSeeds.clear();
        vSeeds.clear();
        // privora test seeds

        vSeeds.push_back(CDNSSeedData("DEVNET1", "devnet1.privora.org", false));
        vSeeds.push_back(CDNSSeedData("DEVNET2", "devnet2.privora.org", false));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 179);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0x8e};   // EXD prefix for the address
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 186);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xD0).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x95).convert_to_container < std::vector < unsigned char > > ();
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_dev, pnSeed6_dev + ARRAYLEN(pnSeed6_dev));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fAllowMultiplePorts = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, uint256S("0x"))
        };

        chainTxData = ChainTxData{
            1414776313,
            0,
            0.001
        };

        // Sigma related values.
        consensus.nSigmaStartBlock = 1;
        consensus.nSigmaPaddingBlock = 1;
        consensus.nDisableUnpaddedSigmaBlock = 1;
        consensus.nStartSigmaBlacklist = INT_MAX;
        consensus.nRestartSigmaWithBlacklistCheck = INT_MAX;
        consensus.nOldSigmaBanBlock = 1;

        consensus.nLelantusStartBlock = 1;
        consensus.nLelantusFixesStartBlock = 1;

        consensus.nSparkStartBlock = 1500;
        consensus.nLelantusGracefulPeriod = 6000;
        consensus.nSigmaEndBlock = 3600;
        consensus.nMaxSigmaInputPerBlock = ZC_SIGMA_INPUT_LIMIT_PER_BLOCK;
        consensus.nMaxValueSigmaSpendPerBlock = ZC_SIGMA_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSigmaInputPerTransaction = ZC_SIGMA_INPUT_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSigmaSpendPerTransaction = ZC_SIGMA_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxLelantusInputPerBlock = ZC_LELANTUS_INPUT_LIMIT_PER_BLOCK;
        consensus.nMaxValueLelantusSpendPerBlock = 1100 * COIN;
        consensus.nMaxLelantusInputPerTransaction = ZC_LELANTUS_INPUT_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueLelantusSpendPerTransaction = 1001 * COIN;
        consensus.nMaxValueLelantusMint = 1001 * COIN;
        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nZerocoinToSigmaRemintWindowSize = 0;

        consensus.evoSporkKeyID = "Tg6CSyHKVTUhMGGNzUQMMDRk88nWW1MdHz";
        consensus.nEvoSporkStartBlock = 1;
        consensus.nEvoSporkStopBlock = 40000;
        consensus.nEvoSporkStopBlockExtensionVersion = 0;

        // reorg
        consensus.nMaxReorgDepth = 4;
        consensus.nMaxReorgDepthEnforcementBlock = 25150;

        // whitelist

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = DANDELION_TESTNET_EMBARGO_MINIMUM;
        consensus.nDandelionEmbargoAvgAdd = DANDELION_TESTNET_EMBARGO_AVG_ADD;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 1;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = 1;

        // exchange address
        consensus.nExchangeAddressStartBlock = 2500;

        // spark names
        consensus.nSparkNamesStartBlock = 3500;
        consensus.nSparkNamesFee = standardSparkNamesFee;
    }
};

static CDevNetParams devNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";

        consensus.chainType = Consensus::chainRegtest;


            SelectParams(CBaseChainParams::MAIN);  // Ensure the global pointer is set



        consensus.nSubsidyHalvingInterval = 25;

        consensus.nMasternodePayout = 40;
        consensus.nDevelopmentFundPercent = 10;

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0xd5,0x88,0x6d,0xb4,0x48,0x48,0x72,0xd2,0x2a,0x0f,0x75,0xf8,0xbb,0x7c,0xe4,0xc0,0x4e,0x27,0x8f,0x47,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0xf2,0x19,0x3b,0xd1,0x69,0x73,0x2c,0x1a,0x8d,0x4f,0x0c,0x7f,0x87,0xd3,0x1b,0xc1,0x68,0xdf,0x98,0x84,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 30;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nMasternodePaymentsStartBlock = 10000;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = INT_MAX;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");
        // Znode code
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes
        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in privora

        // evo znodes
        consensus.DIP0003Height = 500;
        consensus.DIP0003EnforcementHeight = 550;
        consensus.DIP0008Height = 550;
        consensus.nEvoZnodeMinimumConfirmations = 1;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_5_60] = llmq5_60;
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.nLLMQPowTargetSpacing = 1;
        consensus.llmqChainLocks = Consensus::LLMQ_5_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_5_60;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;
        consensus.nInstantSendBlockFilteringStartHeight = 500;

        consensus.nDisableZerocoinStartBlock = INT_MAX;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        std::vector<unsigned char> extraNonce(4);
        extraNonce[0] = 0x02;
        extraNonce[1] = 0x04;
        extraNonce[2] = 0x06;
        extraNonce[3] = 0x08;
        

        genesis = CreateGenesisBlock(1749384000, 1, 0x207fffff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x053a0fe01d0d314afe13b84bdd2eba5f5c76a34e8c1e2a40e69e9c3f028da3df"));
        assert(genesis.hashMerkleRoot == uint256S("0x38b69b8b4093adc0068b3c2cc11744a9fcc92a8bca0f0a056aa544febee84634"));
        assert(genesis.mix_hash == uint256S("0xc50977e27552c2996681e2fdca3d3e2b3923d4f518c48c67bb6c061c5e72b741"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fAllowMultiplePorts = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, uint256S("0x00"))
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };
        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 65);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 178);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0xac};   // EXR prefix for the address
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container < std::vector < unsigned char > > ();

        // Sigma related values.
        consensus.nSigmaStartBlock = 100;
        consensus.nSigmaPaddingBlock = 1;
        consensus.nDisableUnpaddedSigmaBlock = 1;
        consensus.nStartSigmaBlacklist = INT_MAX;
        consensus.nRestartSigmaWithBlacklistCheck = INT_MAX;
        consensus.nOldSigmaBanBlock = 1;
        consensus.nLelantusStartBlock = 400;
        consensus.nLelantusFixesStartBlock = 400;
        consensus.nSparkStartBlock = 1000;
        consensus.nExchangeAddressStartBlock = 1000;
        consensus.nLelantusGracefulPeriod = 1500;
        consensus.nSigmaEndBlock = 1400;
        consensus.nZerocoinV2MintMempoolGracefulPeriod = 1;
        consensus.nZerocoinV2MintGracefulPeriod = 1;
        consensus.nZerocoinV2SpendMempoolGracefulPeriod = 1;
        consensus.nZerocoinV2SpendGracefulPeriod = 1;
        consensus.nMaxSigmaInputPerBlock = ZC_SIGMA_INPUT_LIMIT_PER_BLOCK;
        consensus.nMaxValueSigmaSpendPerBlock = ZC_SIGMA_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSigmaInputPerTransaction = ZC_SIGMA_INPUT_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSigmaSpendPerTransaction = ZC_SIGMA_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxLelantusInputPerBlock = ZC_LELANTUS_INPUT_LIMIT_PER_BLOCK;
        consensus.nMaxValueLelantusSpendPerBlock = ZC_LELANTUS_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxLelantusInputPerTransaction = ZC_LELANTUS_INPUT_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueLelantusSpendPerTransaction = ZC_LELANTUS_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueLelantusMint = ZC_LELANTUS_MAX_MINT;
        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nZerocoinToSigmaRemintWindowSize = 1000;

        // evo spork
        consensus.evoSporkKeyID = "TSpmHGzQT4KJrubWa4N2CRmpA7wKMMWDg4";  // private key is cW2YM2xaeCaebfpKguBahUAgEzLXgSserWRuD29kSyKHq1TTgwRQ
        consensus.nEvoSporkStartBlock = 550;
        consensus.nEvoSporkStopBlock = 950;
        consensus.nEvoSporkStopBlockExtensionVersion = 0;

        // reorg
        consensus.nMaxReorgDepth = 4;
        consensus.nMaxReorgDepthEnforcementBlock = 300;

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = 0;
        consensus.nDandelionEmbargoAvgAdd = 1;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 0;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = 800;

        // spark names
        consensus.nSparkNamesStartBlock = 2000;
        consensus.nSparkNamesFee = standardSparkNamesFee;
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::DEVNET)
            return devNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}

