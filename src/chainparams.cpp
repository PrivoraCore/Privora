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

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x19,0xe8,0xa2,0xd1,0xa2,0x31,0x16,0x5d,0x45,0x0f,0x1c,0x3d,0x26,0x0d,0x9f,0xfc,0x0d,0x7c,0x57,0xa4,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0x7a,0x73,0x87,0x85,0x95,0x71,0xd2,0xff,0xca,0x8a,0x7e,0xf7,0xef,0x15,0x3a,0xce,0x45,0x61,0xf9,0xb8,0x88,0xac};

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
        consensus.defaultAssumeValid = uint256S("0x0007229e4654a3037b5d2a9642a846fdb6a67f519702768d3dd3a678d2dcb20a");

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
        genesis = CreateGenesisBlock(1749384000, 2330, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);

        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);
        assert(consensus.hashGenesisBlock == uint256S("0x0007229e4654a3037b5d2a9642a846fdb6a67f519702768d3dd3a678d2dcb20a"));
        assert(genesis.hashMerkleRoot == uint256S("0x6b4e4f4568408907285bb00dced1103aa1879fdc61bb27b28c20534845e9bbb0"));
        assert(genesis.mix_hash == uint256S("0x3a70e04064c32aeb425812cb9ecbb94af3bd822d44f6d7b3416ae2a9ebc4cfbd"));

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
        consensus.nDisableZerocoinStartBlock = INT_MAX;

        consensus.nZerocoinToSigmaRemintWindowSize = INT_MAX;

        consensus.evoSporkKeyID = "PjJYELp783TABDxhUCD4Xb79PS2EEHYbDb";
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
        consensus.defaultAssumeValid = uint256S("0x000345d9426a940aca152a26c3eb00ede55165dd0d34ab5766fddf6b48293bc6");

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
        genesis = CreateGenesisBlock(1749384000, 843, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x000345d9426a940aca152a26c3eb00ede55165dd0d34ab5766fddf6b48293bc6"));
        assert(genesis.hashMerkleRoot == uint256S("0x86419aef7c300a8757185d77e0e183bed93ff7c0257ddbced1e23cebd4d0e9bb"));
        assert(genesis.mix_hash == uint256S("0xd9da891720e8d94fe7d2e9d257b2cfd7b46ae8a9c918672d358900241db8fd1b"));

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
            (0, uint256S("0x000345d9426a940aca152a26c3eb00ede55165dd0d34ab5766fddf6b48293bc6"))
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

        consensus.evoSporkKeyID = "Tkd96mATUVqpcLHcUQYbiVMkvTsdJDTzaa";
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
        consensus.defaultAssumeValid = uint256S("0x0007b7d63bfa4d68633b66b7c8f35aff2520a925c502821c821fddb9ecf16d7e");

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
        genesis = CreateGenesisBlock(1749384000, 100, 0x1f0affff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x0007b7d63bfa4d68633b66b7c8f35aff2520a925c502821c821fddb9ecf16d7e"));
        assert(genesis.hashMerkleRoot == uint256S("0x0a0eef29fa17cd3358bf81bd1198ca8d87acb74c2fb6b9ddb608300c50dba79e"));
        assert(genesis.mix_hash == uint256S("0xac84b070724809403c015438761e5faeff582a08defd9bcb09df01b9049e5873"));

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
            (0, uint256S("0x0007b7d63bfa4d68633b66b7c8f35aff2520a925c502821c821fddb9ecf16d7e"))
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

        consensus.evoSporkKeyID = "TdFcWhVGVr7VbH4UAgw5gqfRcypPxBzwUh";
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

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0xd5,0x88,0x6d,0xb4,0x48,0x48,0x72,0xd2,0x2a,0x0f,0x75,0xf8,0xbb,0x7c,0xe4,0xc0,0x4e,0x27,0x8f,0x47,0x88,0xac};
        consensus.developmentOutputScriptHex = {0x76,0xa9,0x14,0xf2,0x19,0x3b,0xd1,0x69,0x73,0x2c,0x1a,0x8d,0x4f,0x0c,0x7f,0x87,0xd3,0x1b,0xc1,0x68,0xdf,0x98,0x84,0x88,0xac};

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
        consensus.defaultAssumeValid = uint256S("0x053a0fe01d0d314afe13b84bdd2eba5f5c76a34e8c1e2a40e69e9c3f028da3df");

        pchMessageStart[0] = 0xf4;
        pchMessageStart[1] = 0x0b;
        pchMessageStart[2] = 0xa7;
        pchMessageStart[3] = 0x6c;

        std::vector<unsigned char> extraNonce = {0x02, 0x04, 0x06, 0x08};
        genesis = CreateGenesisBlock(1749384000, 1, 0x207fffff, 2, consensus.genesisOutputScriptHex, consensus.developmentOutputScriptHex, 90 * COIN, 15 * COIN, extraNonce);
        consensus.hashGenesisBlock = genesis.GetHashFull(genesis.mix_hash);

        assert(consensus.hashGenesisBlock == uint256S("0x053a0fe01d0d314afe13b84bdd2eba5f5c76a34e8c1e2a40e69e9c3f028da3df"));
        assert(genesis.hashMerkleRoot == uint256S("0x38b69b8b4093adc0068b3c2cc11744a9fcc92a8bca0f0a056aa544febee84634"));
        assert(genesis.mix_hash == uint256S("0xc50977e27552c2996681e2fdca3d3e2b3923d4f518c48c67bb6c061c5e72b741"));

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
            (0, uint256S("0x053a0fe01d0d314afe13b84bdd2eba5f5c76a34e8c1e2a40e69e9c3f028da3df"))
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
        consensus.evoSporkKeyID = "TfNLtqNtq8JjHW2dhMA1uUoBUUisw864TJ";
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

