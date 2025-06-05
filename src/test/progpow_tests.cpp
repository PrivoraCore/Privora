#include "util.h"

#include "clientversion.h"
#include "primitives/transaction.h"
#include "random.h"
#include "sync.h"
#include "utilstrencodings.h"
#include "utilmoneystr.h"
#include "validation.h"
#include "validationinterface.h"
#include "test/test_privora.h"
#include "script/standard.h"
#include <consensus/merkle.h>

#include <boost/test/unit_test.hpp>
#include <boost/signals2/connection.hpp>

struct ProgpowTestingSetup : public TestChain100Setup
{
    CKey coinbaseKey;
    CScript coinbaseScript;
    Consensus::Params &mutableParams;
    Consensus::Params originalParams;

    class ProgpowValidationInterface : public CValidationInterface {
    public:
        std::string errorCode;

    protected:
        virtual void BlockChecked(const CBlock&, const CValidationState& state) {
            errorCode = state.GetRejectReason();
        }
    };

    ProgpowTestingSetup() : TestChain100Setup(), mutableParams(const_cast<Consensus::Params&>(Params().GetConsensus()))
    {
        originalParams = mutableParams;
        coinbaseKey.MakeNewKey(true);
        coinbaseScript = GetScriptForDestination(coinbaseKey.GetPubKey().GetID());
    }

    ~ProgpowTestingSetup() {
        mutableParams = originalParams;
        SetMockTime(0);
    }

    bool VerifyBlockCheckStatus(const CBlock &block, const std::string &correctError, bool shouldPassPowCheck=true) {
        ProgpowValidationInterface validationInterface;

        RegisterValidationInterface(&validationInterface);
        CBlock newBlock = block;
        // brute force correct nOnce64 value
        CValidationState state;
        const Consensus::Params &params = Params().GetConsensus();
        while (CheckBlockHeader(newBlock, state, params, true) != shouldPassPowCheck)
            newBlock.nNonce64++;
        ProcessNewBlock(Params(), std::make_shared<CBlock>(newBlock), false, nullptr);
        UnregisterValidationInterface(&validationInterface);

        return correctError == validationInterface.errorCode;
    }
};


BOOST_FIXTURE_TEST_SUITE(progpow_tests, ProgpowTestingSetup)

BOOST_AUTO_TEST_CASE(corruption)
{
    SetMockTime(0);

    CBlock block = CreateBlock({}, coinbaseKey);
    BOOST_ASSERT(block.IsProgPow());

    CBlock modifiedBlock = block;

    // modifications made to the block should lead to rejection

    // try to modify transaction for block
    CMutableTransaction coinbaseTx = *block.vtx[0];
    CScript script;
    script << OP_TRUE;
    coinbaseTx.vout[0].scriptPubKey = script;
    modifiedBlock.vtx[0] = std::make_shared<CTransaction>(coinbaseTx);
    modifiedBlock.hashMerkleRoot = BlockMerkleRoot(modifiedBlock);
    BOOST_ASSERT(VerifyBlockCheckStatus(modifiedBlock, "invalid-mixhash"));

    // try to modify field in header
    modifiedBlock = block;
    modifiedBlock.nTime++;
    BOOST_ASSERT(VerifyBlockCheckStatus(modifiedBlock, "invalid-mixhash"));

    // try to modify nHeight
    modifiedBlock = block;
    modifiedBlock.nHeight++;
    BOOST_ASSERT(VerifyBlockCheckStatus(modifiedBlock, "bad-blk-progpow"));

    // try to modify nNonce64 so the block wouldn't pass PoW check
    modifiedBlock = block;
    BOOST_ASSERT(VerifyBlockCheckStatus(modifiedBlock, "high-hash", false));

    // try to modify mix_hash
    modifiedBlock = block;
    modifiedBlock.mix_hash = uint256S("e2c490c5bf0c3810cb8216997653d937b2822c1da37868b7e5f3c1a380c8a80a"); // SHA256("Privora")
    BOOST_ASSERT(VerifyBlockCheckStatus(modifiedBlock, "invalid-mixhash"));

    // verify that unmodified block passes all the checks
    BOOST_ASSERT(VerifyBlockCheckStatus(block, ""));
}

BOOST_AUTO_TEST_SUITE_END()
