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

static CBlock CreateGenesisBlock(uint32_t nTime, uint64_t nNonce, uint32_t nBits, int32_t nVersion, const std::vector<uint8_t> &genesisOutputScriptHex, const std::vector<uint8_t> &developmentOutputScriptHex, const CAmount &genesisReward, const CAmount &developmentReward, std::vector<unsigned char> extraNonce) {
    const char *pszTimestamp = "CNN - 08/06/2025 - Trump deploys National Guard to Los Angeles amid immigration protests.";

    const CScript genesisOutputScript = CScript(genesisOutputScriptHex.begin(), genesisOutputScriptHex.end());
    const CScript developmentOutputScript = CScript(developmentOutputScriptHex.begin(), developmentOutputScriptHex.end());

    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, developmentOutputScript, nTime, nNonce, nBits, nVersion, genesisReward, developmentReward, extraNonce);
}

static Consensus::LLMQParams llmq5_60 = {
        .type = Consensus::LLMQ_5_60,
        .name = "llmq_5_60",
        .size = 5,
        .minSize = 3,
        .threshold = 3,
        .dkgInterval = 24, 
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10,
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 8,
        .signingActiveQuorumCount = 2,
        .keepOldConnections = 3,
};

static Consensus::LLMQParams llmq10_70 = {
        .type = Consensus::LLMQ_10_70,
        .name = "llmq_10_70",
        .size = 10,
        .minSize = 8,
        .threshold = 7,
        .dkgInterval = 24,
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10,
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 8,
        .signingActiveQuorumCount = 2,
        .keepOldConnections = 3,
};

static Consensus::LLMQParams llmq50_60 = {
        .type = Consensus::LLMQ_50_60,
        .name = "llmq_50_60",
        .size = 50,
        .minSize = 40,
        .threshold = 30,
        .dkgInterval = 18,
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10,
        .dkgMiningWindowEnd = 16,
        .dkgBadVotesThreshold = 40,
        .signingActiveQuorumCount = 16,
        .keepOldConnections = 17,
};

static Consensus::LLMQParams llmq400_60 = {
        .type = Consensus::LLMQ_400_60,
        .name = "llmq_400_60",
        .size = 400,
        .minSize = 300,
        .threshold = 240,
        .dkgInterval = 12 * 12,
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, 
        .dkgMiningWindowEnd = 28,
        .dkgBadVotesThreshold = 300,
        .signingActiveQuorumCount = 4,
        .keepOldConnections = 5,
};

static Consensus::LLMQParams llmq400_85 = {
        .type = Consensus::LLMQ_400_85,
        .name = "llmq_400_85",
        .size = 400,
        .minSize = 350,
        .threshold = 340,
        .dkgInterval = 12 * 24,
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20,
        .dkgMiningWindowEnd = 48,
        .dkgBadVotesThreshold = 300,
        .signingActiveQuorumCount = 4,
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

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x59,0x52,0x68,0xb1,0x2c,0x5b,0x5b,0x27,0x1c,0xd4,0x08,0x3f,0x42,0x10,0x9a,0x94,0xfa,0x27,0x0f,0x56,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0x53,0xf1,0x91,0x7d,0x9c,0x12,0x87,0x48,0xc4,0x3c,0x19,0xab,0xa4,0xba,0x28,0xdb,0xe3,0xa1,0xdd,0x92,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = INT_MAX;
        consensus.BIP66Height = INT_MAX;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 30;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1475020800;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1479168000;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1510704000;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000001745");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000460385688f4b6743d03fcda2f9f7ff8d6179af91f2314cdc4cdc2e8277364");

        consensus.nMasternodePaymentsStartBlock = 10000;

        // evo znodes
        consensus.DIP0003Height = 100;
        consensus.DIP0003EnforcementHeight = 110;
        consensus.DIP0008Height = 120;
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
        consensus.nInstantSendBlockFilteringStartHeight = 1;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xa3;
        pchMessageStart[1] = 0x1f;
        pchMessageStart[2] = 0xc9;
        pchMessageStart[3] = 0x77;

        std::vector<unsigned char> extraNonce = {0x34, 0xab, 0x6c, 0xfe};
        genesis = CreateGenesisBlock(1749384000, 2675, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x000460385688f4b6743d03fcda2f9f7ff8d6179af91f2314cdc4cdc2e8277364"));
        assert(genesis.hashMerkleRoot == uint256S("0x088a0d227113b9c2cf6d306e051ecbc7c77e65f2944bbcdbb172a425a50d8163"));
        assert(genesis.mix_hash == uint256S("0x255797b21d14b0c1a7f180f4af669dab55b10fff290fca824eedf8bb23fa2501"));

        vSeeds.push_back(CDNSSeedData("dns-mainnet-1.privora.org", "dns-mainnet-1.privora.org", false));
        vSeeds.push_back(CDNSSeedData("dns-mainnet-2.privora.org", "dns-mainnet-2.privora.org", false));

        // Note that of those with the service bits flag, most only support a subset of possible options
        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 56);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 63);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0xbb};
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 210);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container < std::vector < unsigned char > > ();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        nDefaultPort = 5660;
        nPruneAfterHeight = 100000;
        nMaxTipAge = 6 * 60 * 60;
        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60;

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fAllowMultiplePorts = false;

        checkpointData = (CCheckpointData) {
                boost::assign::map_list_of
                (0, uint256S("0x000460385688f4b6743d03fcda2f9f7ff8d6179af91f2314cdc4cdc2e8277364"))
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
        consensus.nDisableZerocoinStartBlock = INT_MAX;

        consensus.nZerocoinToSigmaRemintWindowSize = INT_MAX;

        consensus.evoSporkKeyID = "PfadcTvPhr8L9k6jaRbXT1mwgp8U8jXcp1";
        consensus.nEvoSporkStartBlock = 1;
        consensus.nEvoSporkStopBlock = 0;
        consensus.nEvoSporkStopBlockExtensionVersion = 0;
        consensus.nEvoSporkStopBlockPrevious = 1 + 1*24*12*365; 
        consensus.nEvoSporkStopBlockExtensionGracefulPeriod = 24*12*14;

        // reorg
        consensus.nMaxReorgDepth = 5;
        consensus.nMaxReorgDepthEnforcementBlock = 1;

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = DANDELION_EMBARGO_MINIMUM;
        consensus.nDandelionEmbargoAvgAdd = DANDELION_EMBARGO_AVG_ADD;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 1;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = INT_MAX;

        // exchange address
        consensus.nExchangeAddressStartBlock = consensus.nSparkStartBlock;

        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nSparkStartBlock = 100;

        // spark names
        consensus.nSparkNamesStartBlock = 105; 
        consensus.nSparkNamesFee = standardSparkNamesFee;
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

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0xcc,0xe3,0xe3,0xa9,0x9b,0xa4,0x56,0xb5,0x34,0x5c,0x71,0x13,0x43,0xd1,0x33,0x85,0xba,0x10,0x4e,0xb6,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0xf6,0x9e,0x1c,0xa9,0xc5,0xaa,0xa6,0x5c,0x14,0xa6,0xbf,0xdc,0x2e,0x81,0x57,0x0a,0xa9,0xd4,0x7a,0xf5,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = INT_MAX;
        consensus.BIP66Height = INT_MAX;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 30;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000001745");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0005ae521ee504a91b4c012c1fd75b0228bf057b482475a3f2db137963b6d484");

        consensus.nMasternodePaymentsStartBlock = 10000;

        // evo znodes
        consensus.DIP0003Height = 100;
        consensus.DIP0003EnforcementHeight = 110;
        consensus.DIP0008Height = 120;
        consensus.nEvoZnodeMinimumConfirmations = 15;

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
        consensus.nInstantSendBlockFilteringStartHeight = 1;

        pchMessageStart[0] = 0x6b;
        pchMessageStart[1] = 0x2d;
        pchMessageStart[2] = 0x8e;
        pchMessageStart[3] = 0x40;

        std::vector<unsigned char> extraNonce = { 0xac, 0xb3, 0x21, 0x64 };
        genesis = CreateGenesisBlock(1749384000, 2222, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x0005ae521ee504a91b4c012c1fd75b0228bf057b482475a3f2db137963b6d484"));
        assert(genesis.hashMerkleRoot == uint256S("0xed280e0df72772a30cfc9869453ca382a181f956c541803d74f58325cec5bbe1"));
        assert(genesis.mix_hash == uint256S("0x36e0658c32f63dceb0d9066a958454307974c0d7fe9e8a7389fc50859365b68f"));

        vFixedSeeds.clear();
        vSeeds.clear();

        vSeeds.push_back(CDNSSeedData("dns-testnet-1.privora.org", "dns-testnet-1.privora.org", false));
        vSeeds.push_back(CDNSSeedData("dns-testnet-2.privora.org", "dns-testnet-2.privora.org", false));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 61);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0xb1};
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 185);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container < std::vector < unsigned char > > ();
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        nDefaultPort = 15660;
        nPruneAfterHeight = 1000;
        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60;
        nMaxTipAge = 0x7fffffff;

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fAllowMultiplePorts = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, uint256S("0x0005ae521ee504a91b4c012c1fd75b0228bf057b482475a3f2db137963b6d484"))
        };

        chainTxData = ChainTxData{
            1414776313,
            0,
            0.001
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
        consensus.nZerocoinToSigmaRemintWindowSize = INT_MAX;
        consensus.nDisableZerocoinStartBlock = INT_MAX;

        consensus.evoSporkKeyID = "TwnodAoZDovgCPq14Qif3EzeYj7FEF4vqf";
        consensus.nEvoSporkStartBlock = 22000;
        consensus.nEvoSporkStopBlock = 40000;
        consensus.nEvoSporkStopBlockExtensionVersion = 0;

        // reorg
        consensus.nMaxReorgDepth = 4;
        consensus.nMaxReorgDepthEnforcementBlock = 25150;

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = DANDELION_TESTNET_EMBARGO_MINIMUM;
        consensus.nDandelionEmbargoAvgAdd = DANDELION_TESTNET_EMBARGO_AVG_ADD;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 1;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = INT_MAX;

        // exchange address
        consensus.nExchangeAddressStartBlock = consensus.nSparkStartBlock;

        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nSparkStartBlock = 100;

        // spark names
        consensus.nSparkNamesStartBlock = 105; 
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

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x8d,0xb1,0xc0,0x8a,0x11,0x7e,0xa1,0xc0,0xca,0x87,0xdf,0xab,0x4d,0x25,0x4d,0xf9,0x1c,0x8d,0x0d,0x42,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0x4d,0x82,0x9d,0x11,0xa9,0x0e,0x07,0x4a,0xb3,0xd8,0x3e,0x15,0xf4,0x00,0xf4,0x87,0xc1,0xf6,0x03,0x27,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = INT_MAX;
        consensus.BIP66Height = INT_MAX;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 30;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000001745");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0003b9c73fd3a75bafce67b7151eb182272609a8682bccbfd3ffa96cfa70790e");

        // Znode params testnet
        consensus.nMasternodePaymentsStartBlock = 10000;

        // evo znodes
        consensus.DIP0003Height = 100;
        consensus.DIP0003EnforcementHeight = 110;
        consensus.DIP0008Height = 120;
        consensus.nEvoZnodeMinimumConfirmations = 15;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_5_60] = llmq5_60;
        consensus.llmqs[Consensus::LLMQ_10_70] = llmq10_70;
        consensus.nLLMQPowTargetSpacing = 20;
        consensus.llmqChainLocks = Consensus::LLMQ_5_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_5_60;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;
        consensus.nInstantSendBlockFilteringStartHeight = 1;
        
        pchMessageStart[0] = 0x5e;
        pchMessageStart[1] = 0x99;
        pchMessageStart[2] = 0xd3;
        pchMessageStart[3] = 0x14;

        std::vector<unsigned char> extraNonce = { 0x0f, 0xd4, 0x7c, 0x35 };
        genesis = CreateGenesisBlock(1749384000, 3314, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x0003b9c73fd3a75bafce67b7151eb182272609a8682bccbfd3ffa96cfa70790e"));
        assert(genesis.hashMerkleRoot == uint256S("0x4d3a4ef78d054e41ad581e1fd70763d7252f2b1e39ae9fc2cd6a6956e5b8500d"));
        assert(genesis.mix_hash == uint256S("0x79d4c419de713143ea68ef7fa6ebed99d912e5d0a1754854a31bf8c1b0dd0d4c"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // privora test seeds

        vSeeds.push_back(CDNSSeedData("dns-devnet-1.privora.org", "dns-devnet-1.privora.org", false));
        vSeeds.push_back(CDNSSeedData("dns-devnet-2.privora.org", "dns-devnet-2.privora.org", false));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 61);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0x8e};
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 186);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xD0).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x95).convert_to_container < std::vector < unsigned char > > ();
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_dev, pnSeed6_dev + ARRAYLEN(pnSeed6_dev));

        nDefaultPort = 25660;
        nPruneAfterHeight = 1000;
        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60;
        nMaxTipAge = 0x7fffffff;

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fAllowMultiplePorts = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, uint256S("0x0003b9c73fd3a75bafce67b7151eb182272609a8682bccbfd3ffa96cfa70790e"))
        };

        chainTxData = ChainTxData{
            1414776313,
            0,
            0.001
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
        consensus.nMaxSigmaInputPerBlock = INT_MAX;
        consensus.nMaxValueSigmaSpendPerBlock = INT_MAX;
        consensus.nMaxSigmaInputPerTransaction = INT_MAX;
        consensus.nMaxValueSigmaSpendPerTransaction = INT_MAX;
        consensus.nMaxLelantusInputPerBlock = INT_MAX;
        consensus.nMaxValueLelantusSpendPerBlock = INT_MAX;
        consensus.nMaxLelantusInputPerTransaction = INT_MAX;
        consensus.nMaxValueLelantusSpendPerTransaction = INT_MAX;
        consensus.nMaxValueLelantusMint = INT_MAX;
        consensus.nZerocoinToSigmaRemintWindowSize = INT_MAX;

        consensus.evoSporkKeyID = "TgNeV8KvhbCm6ebcMCg971qQfN5JgqjPQN";
        consensus.nEvoSporkStartBlock = 1;
        consensus.nEvoSporkStopBlock = 40000;
        consensus.nEvoSporkStopBlockExtensionVersion = 0;

        // reorg
        consensus.nMaxReorgDepth = 4;
        consensus.nMaxReorgDepthEnforcementBlock = 25150;

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = DANDELION_TESTNET_EMBARGO_MINIMUM;
        consensus.nDandelionEmbargoAvgAdd = DANDELION_TESTNET_EMBARGO_AVG_ADD;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 1;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = INT_MAX;

        // exchange address
        consensus.nExchangeAddressStartBlock = consensus.nSparkStartBlock;

        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nSparkStartBlock = 100;

        // spark names
        consensus.nSparkNamesStartBlock = 105; 
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
        consensus.nSubsidyHalvingInterval = 25;

        consensus.nMasternodePayout = 40;
        consensus.nDevelopmentFundPercent = 10;

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x05,0xf7,0x77,0x70,0x36,0x92,0xb6,0xba,0xc7,0xee,0x13,0x72,0x64,0xee,0x7a,0x0d,0xca,0xbd,0xf6,0x8f,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0xbd,0x94,0xb6,0xd2,0x3f,0xd9,0xc8,0x22,0x0c,0x94,0xb3,0x95,0xf7,0xa7,0xb5,0xe9,0xe2,0x5c,0x18,0x2a,0x88,0xac};

        consensus.nStartDuplicationCheck = 0;

        consensus.BIP65Height = 1351;
        consensus.BIP66Height = 1251;
        consensus.powLimit = uint256S("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 30;
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108;
        consensus.nMinerConfirmationWindow = 144;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = INT_MAX;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // evo znodes
        consensus.DIP0003Height = 100;
        consensus.DIP0003EnforcementHeight = 110;
        consensus.DIP0008Height = 120;
        consensus.nEvoZnodeMinimumConfirmations = 15;

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
        consensus.nInstantSendBlockFilteringStartHeight = 1;

        consensus.nMasternodePaymentsStartBlock = 150;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000000000000000000002");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x5d69514c1f578cf29fdc475267d41578861f5c67578d1b288496a2a2b03360c8");

        pchMessageStart[0] = 0xf4;
        pchMessageStart[1] = 0x0b;
        pchMessageStart[2] = 0xa7;
        pchMessageStart[3] = 0x6c;

        std::vector<unsigned char> extraNonce = {0x02, 0x04, 0x06, 0x08};
        genesis = CreateGenesisBlock(1749384000, 2, 0x207fffff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x5d69514c1f578cf29fdc475267d41578861f5c67578d1b288496a2a2b03360c8"));
        assert(genesis.hashMerkleRoot == uint256S("0xb5cf6abce03bf6d7955330313d4ddd0a349b7aed58e248deccdda9e2a1c278af"));
        assert(genesis.mix_hash == uint256S("0x8ed1d8ec91bedfeb65bb3687c5c48960c1ee1416db827a94ff2795a1ea02e5e5"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        nDefaultPort = 35660;
        nPruneAfterHeight = 1000;
        nFulfilledRequestExpireTime = 5*60;
        nMaxTipAge = 6 * 60 * 60;

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fAllowMultiplePorts = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, uint256S("0x5d69514c1f578cf29fdc475267d41578861f5c67578d1b288496a2a2b03360c8"))
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };
        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 61);
        base58Prefixes[EXCHANGE_PUBKEY_ADDRESS] = {0x01, 0xb9, 0xac}; 
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container < std::vector < unsigned char > > ();

        // Sigma related values.
        consensus.nSigmaStartBlock = INT_MAX;
        consensus.nSigmaPaddingBlock = INT_MAX;
        consensus.nDisableUnpaddedSigmaBlock = INT_MAX;
        consensus.nStartSigmaBlacklist = INT_MAX;
        consensus.nRestartSigmaWithBlacklistCheck = INT_MAX;
        consensus.nOldSigmaBanBlock = INT_MAX;
        consensus.nLelantusStartBlock = INT_MAX;
        consensus.nLelantusFixesStartBlock = INT_MAX;
        consensus.nExchangeAddressStartBlock = INT_MAX;
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
        consensus.nZerocoinToSigmaRemintWindowSize = INT_MAX;
        consensus.nDisableZerocoinStartBlock = INT_MAX;

        // evo spork
        consensus.evoSporkKeyID = "TrbDrGEk8chbH3s1QwRDWyKrtRYxHXRzbz";
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
        consensus.nMnemonicBlock = 1;

        // moving lelantus data to v3 payload
        consensus.nLelantusV3PayloadStartBlock = INT_MAX;

        // exchange address
        consensus.nExchangeAddressStartBlock = consensus.nSparkStartBlock;

        consensus.nMaxValueSparkSpendPerTransaction = SPARK_VALUE_SPEND_LIMIT_PER_TRANSACTION;
        consensus.nMaxValueSparkSpendPerBlock = SPARK_VALUE_SPEND_LIMIT_PER_BLOCK;
        consensus.nMaxSparkOutLimitPerTx = SPARK_OUT_LIMIT_PER_TX;
        consensus.nSparkStartBlock = 100;

        // spark names
        consensus.nSparkNamesStartBlock = 105; 
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

