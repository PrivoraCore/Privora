#!/usr/bin/env python3
# Copyright (c) 2015-2018 The Dash Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Test deterministic masternodes
#

from test_framework.blocktools import create_block, create_coinbase, get_masternode_payment, get_founders_rewards
from test_framework.mininode import CTransaction, ToHex, FromHex, CTxOut, COIN, CCbTx
from test_framework.test_framework import PrivoraTestFramework
from test_framework.util import *
from test_framework.mn_utils import *

class DIP3Test(PrivoraTestFramework):
    def __init__(self):
        super().__init__()
        self.num_initial_mn = 11 # Should be >= 11 to make sure quorums are not always the same MNs
        self.num_nodes = 1 + self.num_initial_mn + 2 # +1 for controller, +1 for mn-qt, +1 for mn created after dip3 activation
        self.setup_clean_chain = True

    def setup_network(self):
        disable_mocktime()
        self.start_controller_node()
        self.is_network_split = False

    def start_controller_node(self, extra_args=None):
        self.log.info("starting controller node")
        if self.nodes is None:
            self.nodes = [None]
        args = []
        if extra_args is not None:
            args += extra_args
        self.nodes[0] = start_node(0, self.options.tmpdir, extra_args=args)
        for i in range(1, self.num_nodes):
            if i < len(self.nodes) and self.nodes[i] is not None:
                connect_nodes_bi(self.nodes, 0, i)

    def stop_controller_node(self):
        self.log.info("stopping controller node")
        stop_node(self.nodes[0], 0)

    def restart_controller_node(self):
        self.stop_controller_node()
        self.start_controller_node()

    def run_test(self):
        self.log.info("funding controller node")
        while self.nodes[0].getbalance() < (self.num_initial_mn + 3) * 1000:
            self.nodes[0].generate(1) # generate enough for collaterals
        self.log.info("controller node has {} PRIVORA".format(self.nodes[0].getbalance()))

        # Make sure we're below block 135 (which activates dip3)
        self.log.info("testing rejection of ProTx before dip3 activation")
        assert(self.nodes[0].getblockchaininfo()['blocks'] < 500)

        mns = []

        # prepare mn which should still be accepted later when dip3 activates
        self.log.info("creating collateral for mn-before-dip3")
        before_dip3_mn = prepare_mn(self.nodes[0], 1, 'mn-before-dip3')
        create_mn_collateral(self.nodes[0], before_dip3_mn)
        mns.append(before_dip3_mn)

        # block 550 starts enforcing DIP3 MN payments
        while self.nodes[0].getblockcount() < 550:
            self.nodes[0].generate(1)

        self.log.info("mining final block for DIP3 activation")
        self.nodes[0].generate(1)

        # We have hundreds of blocks to sync here, give it more time
        self.log.info("syncing blocks for all nodes")
        sync_blocks(self.nodes, timeout=120)

        # DIP3 is fully enforced here
        register_mn(self.nodes[0], before_dip3_mn)
        start_mn(self, before_dip3_mn)

        self.log.info("registering MNs")
        for i in range(0, self.num_initial_mn):
            mn = prepare_mn(self.nodes[0], i + 2, "mn-%d" % i)
            mns.append(mn)

            # start a few MNs before they are registered and a few after they are registered
            start = (i % 3) == 0
            if start:
                start_mn(self, mn)

            # let a few of the protx MNs refer to the existing collaterals
            fund = (i % 2) == 0
            if fund:
                self.log.info("register_fund %s" % mn.alias)
                register_fund_mn(self.nodes[0], mn)
            else:
                self.log.info("create_collateral %s" % mn.alias)
                create_mn_collateral(self.nodes[0], mn)
                self.log.info("register %s" % mn.alias)
                register_mn(self.nodes[0], mn)

            self.nodes[0].generate(1)

            if not start:
                start_mn(self, mn)

            self.sync_all()
            self.assert_mnlists(mns)

#        self.log.info("testing instant send")
#        self.test_instantsend(10, 3)

        self.nodes[1].generate(1)
        self.sync_all()

        self.log.info("test mn payment enforcement with deterministic MNs")
        for i in range(20):
            node = self.nodes[i % len(self.nodes)]
            self.test_invalid_mn_payment(node)
            node.generate(1)
            self.sync_all()

        self.log.info("testing ProUpServTx")
        for mn in mns:
            self.test_protx_update_service(mn)

        self.log.info("testing P2SH/multisig for payee addresses")
        multisig = self.nodes[0].createmultisig(1, [self.nodes[0].getnewaddress(), self.nodes[0].getnewaddress()])['address']
        self.update_mn_payee(mns[0], multisig)
        found_multisig_payee = False
        for i in range(len(mns)):
            bt = self.nodes[0].getblocktemplate()
            expected_payee = bt['znode'][0]['payee']
            expected_amount = bt['znode'][0]['amount'] / COIN
            self.nodes[0].generate(1)
            self.sync_all()
            if expected_payee == multisig:
                block = self.nodes[0].getblock(self.nodes[0].getbestblockhash())
                cbtx = self.nodes[0].getrawtransaction(block['tx'][0], 1)
                for out in cbtx['vout']:
                    if 'addresses' in out['scriptPubKey']:
                        if expected_payee in out['scriptPubKey']['addresses'] and out['value'] == expected_amount:
                            found_multisig_payee = True
        assert(found_multisig_payee)

        self.log.info("testing reusing of collaterals for replaced MNs")
        for i in range(0, 5):
            mn = mns[i]
            # a few of these will actually refer to old ProRegTx internal collaterals,
            # which should work the same as external collaterals
            new_mn = prepare_mn(self.nodes[0], mn.idx, mn.alias)
            new_mn.collateral_address = mn.collateral_address
            new_mn.collateral_txid = mn.collateral_txid
            new_mn.collateral_vout = mn.collateral_vout

            register_mn(self.nodes[0], new_mn)
            mns[i] = new_mn
            self.nodes[0].generate(1)
            self.sync_all()
            self.assert_mnlists(mns)
            self.log.info("restarting MN %s" % new_mn.alias)
            self.stop_node(new_mn.idx)
            start_mn(self, new_mn)
            self.sync_all()

        self.log.info("test that MNs disappear from the list when the ProTx collateral is spent")
        spend_mns_count = 3
        mns_tmp = [] + mns
        dummy_txins = []
        for i in range(spend_mns_count):
            dummy_txin = self.spend_mn_collateral(mns[i], with_dummy_input_output=True)
            dummy_txins.append(dummy_txin)
            self.nodes[0].generate(1)
            self.sync_all()
            mns_tmp.remove(mns[i])
            self.assert_mnlists(mns_tmp)

        self.log.info("test that reverting the blockchain on a single node results in the mnlist to be reverted as well")
        for i in range(spend_mns_count):
            self.nodes[0].invalidateblock(self.nodes[0].getbestblockhash())
            mns_tmp.append(mns[spend_mns_count - 1 - i])
            self.assert_mnlist(self.nodes[0], mns_tmp)

        # self.log.info("cause a reorg with a double spend and check that mnlists are still correct on all nodes")
        # self.mine_double_spend(self.nodes[0], dummy_txins, self.nodes[0].getnewaddress(), use_mnmerkleroot_from_tip=True)
        # self.nodes[0].generate(spend_mns_count)
        # self.sync_all()
        # self.assert_mnlists(mns_tmp)
        #
        # self.log.info("testing instant send with replaced MNs")
        # self.test_instantsend(10, 3, timeout=20)

    def spend_mn_collateral(self, mn, with_dummy_input_output=False):
        return self.spend_input(mn.collateral_txid, mn.collateral_vout, 1000, with_dummy_input_output)

    def update_mn_payee(self, mn, payee):
        self.nodes[0].sendtoaddress(mn.fundsAddr, 0.001)
        self.nodes[0].protx('update_registrar', mn.protx_hash, '', '', payee, mn.fundsAddr)
        self.nodes[0].generate(1)
        self.sync_all()
        info = self.nodes[0].protx('info', mn.protx_hash)
        assert(info['state']['payoutAddress'] == payee)

    def test_protx_update_service(self, mn):
        self.nodes[0].sendtoaddress(mn.fundsAddr, 0.001)
        self.nodes[0].protx('update_service', mn.protx_hash, '127.0.0.2:%d' % mn.p2p_port, mn.blsMnkey, "", mn.fundsAddr)
        self.nodes[0].generate(1)
        self.sync_all()
        for node in self.nodes:
            protx_info = node.protx('info', mn.protx_hash)
            mn_list = node.evoznode('list')
            assert_equal(protx_info['state']['service'], '127.0.0.2:%d' % mn.p2p_port)
            assert_equal(mn_list['COutPoint(%s, %d)' % (mn.collateral_txid, mn.collateral_vout)]['address'], '127.0.0.2:%d' % mn.p2p_port)

        # undo
        self.nodes[0].protx('update_service', mn.protx_hash, '127.0.0.1:%d' % mn.p2p_port, mn.blsMnkey, "", mn.fundsAddr)
        self.nodes[0].generate(1)

    def force_finish_mnsync(self, node):
        tm = 0
        while tm < 30:
            s = node.evoznsync('next')
            if s == 'sync updated to MASTERNODE_SYNC_FINISHED' \
                    or s == 'sync updated to ZNODE_SYNC_FINISHED':
                break
            time.sleep(0.1)
            tm += 0.1

    def force_finish_mnsync_list(self, node):
        if node.evoznsync('status')['AssetName'] == 'MASTERNODE_SYNC_WAITING'\
                or node.evoznsync('status')['AssetName'] == 'ZNODE_SYNC_WAITING':
            node.evoznsync('next')

        tm = 0
        while tm < 30:
            mnlist = node.evoznode('list', 'status')
            if len(mnlist) != 0:
                time.sleep(0.5)
                self.force_finish_mnsync(node)
                return
            time.sleep(0.1)
            tm += 0.1

    def test_instantsend(self, tx_count, repeat, timeout=20):
        self.nodes[0].spork('SPORK_2_INSTANTSEND_ENABLED', 0)
        self.wait_for_sporks()

        # give all nodes some coins first
        for i in range(tx_count):
            outputs = {}
            for node in self.nodes[1:]:
                outputs[node.getnewaddress()] = 1
            rawtx = self.nodes[0].createrawtransaction([], outputs)
            rawtx = self.nodes[0].fundrawtransaction(rawtx)['hex']
            rawtx = self.nodes[0].signrawtransaction(rawtx)['hex']
            self.nodes[0].sendrawtransaction(rawtx)
            self.nodes[0].generate(1)
        self.sync_all()

        for j in range(repeat):
            for i in range(tx_count):
                while True:
                    from_node_idx = random.randint(1, len(self.nodes) - 1)
                    from_node = self.nodes[from_node_idx]
                    if from_node is not None:
                        break
                while True:
                    to_node_idx = random.randint(0, len(self.nodes) - 1)
                    to_node = self.nodes[to_node_idx]
                    if to_node is not None and from_node is not to_node:
                        break
                to_address = to_node.getnewaddress()
                txid = from_node.instantsendtoaddress(to_address, 0.1)
                for node in self.nodes:
                    if node is not None:
                        self.wait_for_instant_lock(node, to_node_idx, txid, timeout=timeout)
            self.nodes[0].generate(6)
            self.sync_all()

    def wait_for_instant_lock(self, node, node_idx, txid, timeout=10):
        st = time.time()
        while time.time() < st + timeout:
            try:
                tx = node.getrawtransaction(txid, 1)
            except:
                tx = None
            if tx is None:
                time.sleep(0.5)
                continue
            if tx['instantlock']:
                return
            time.sleep(0.5)
        raise AssertionError("wait_for_instant_lock timed out for: {} on node {}".format(txid, node_idx))

    def assert_mnlists(self, mns):
        for node in self.nodes:
            self.assert_mnlist(node, mns)

    def assert_mnlist(self, node, mns):
        if not self.compare_mnlist(node, mns):
            expected = []
            for mn in mns:
                expected.append('%s, %d' % (mn.collateral_txid, mn.collateral_vout))
            self.log.error('mnlist: ' + str(node.evoznode('list', 'status')))
            self.log.error('expected: ' + str(expected))
            raise AssertionError("mnlists does not match provided mns")

    def wait_for_sporks(self, timeout=30):
        st = time.time()
        while time.time() < st + timeout:
            if self.compare_sporks():
                return
            time.sleep(0.5)
        raise AssertionError("wait_for_sporks timed out")

    def compare_sporks(self):
        sporks = self.nodes[0].spork('show')
        for node in self.nodes[1:]:
            sporks2 = node.spork('show')
            if sporks != sporks2:
                return False
        return True

    def compare_mnlist(self, node, mns):
        mnlist = node.evoznode('list', 'status')
        for mn in mns:
            s = 'COutPoint(%s, %d)' % (mn.collateral_txid, mn.collateral_vout)
            in_list = s in mnlist
            if not in_list:
                return False
            mnlist.pop(s, None)
        if len(mnlist) != 0:
            return False
        return True

    def spend_input(self, txid, vout, amount, with_dummy_input_output=False):
        # with_dummy_input_output is useful if you want to test reorgs with double spends of the TX without touching the actual txid/vout
        address = self.nodes[0].getnewaddress()

        txins = [
            {'txid': txid, 'vout': vout}
        ]
        targets = {address: amount}

        dummy_txin = None
        if with_dummy_input_output:
            dummyaddress = self.nodes[0].getnewaddress()
            unspent = self.nodes[0].listunspent(110)
            for u in unspent:
                if u['amount'] > Decimal(1):
                    dummy_txin = {'txid': u['txid'], 'vout': u['vout']}
                    txins.append(dummy_txin)
                    targets[dummyaddress] = float(u['amount'] - Decimal(0.0001))
                    break

        rawtx = self.nodes[0].createrawtransaction(txins, targets)
        rawtx = self.nodes[0].fundrawtransaction(rawtx)['hex']
        rawtx = self.nodes[0].signrawtransaction(rawtx)['hex']
        new_txid = self.nodes[0].sendrawtransaction(rawtx)

        return dummy_txin

    def mine_block(self, node, vtx=[], miner_address=None, mn_payee=None, mn_amount=None, use_mnmerkleroot_from_tip=False, expected_error=None):
        bt = node.getblocktemplate()
        height = bt['height']
        tip_hash = bt['previousblockhash']

        tip_block = node.getblock(tip_hash)

        coinbasevalue = bt['coinbasevalue']
        if miner_address is None:
            miner_address = node.getnewaddress()
        if mn_payee is None:
            if isinstance(bt['znode'], list):
                mn_payee = bt['znode'][0]['payee']
            else:
                mn_payee = bt['znode']['payee']
        # we can't take the masternode payee amount from the template here as we might have additional fees in vtx

        # calculate fees that the block template included (we'll have to remove it from the coinbase as we won't
        # include the template's transactions
        bt_fees = 0
        for tx in bt['transactions']:
            bt_fees += tx['fee']

        new_fees = 0
        for tx in vtx:
            in_value = 0
            out_value = 0
            for txin in tx.vin:
                txout = node.gettxout("%064x" % txin.prevout.hash, txin.prevout.n, False)
                in_value += int(txout['value'] * COIN)
            for txout in tx.vout:
                out_value += txout.nValue
            new_fees += in_value - out_value

        # fix fees
        coinbasevalue -= bt_fees
        coinbasevalue += new_fees

        if mn_amount is None:
            mn_amount = get_masternode_payment(height, bt['coinbasevalue'])

        outputs = {miner_address: str(Decimal(coinbasevalue) / COIN)}
        if mn_amount > 0:
            outputs[mn_payee] = str(Decimal(mn_amount) / COIN)
        outputs.update(get_founders_rewards(height))

        coinbase = FromHex(CTransaction(), node.createrawtransaction([], outputs))
        coinbase.vin = create_coinbase(height).vin

        # We can't really use this one as it would result in invalid merkle roots for masternode lists
        if len(bt['coinbase_payload']) != 0:
            cbtx = FromHex(CCbTx(version=1), bt['coinbase_payload'])
            if use_mnmerkleroot_from_tip:
                if 'cbTx' in tip_block:
                    cbtx.merkleRootMNList = int(tip_block['cbTx']['merkleRootMNList'], 16)
                else:
                    cbtx.merkleRootMNList = 0
            coinbase.nVersion = 3
            coinbase.nType = 5 # CbTx
            coinbase.vExtraPayload = cbtx.serialize()

        coinbase.calc_sha256()

        block = create_block(int(tip_hash, 16), coinbase)
        block.vtx += vtx

        # Add quorum commitments from template
        for tx in bt['transactions']:
            tx2 = FromHex(CTransaction(), tx['data'])
            if tx2.nType == 6:
                block.vtx.append(tx2)

        block.hashMerkleRoot = block.calc_merkle_root()
        block.solve()
        result = node.submitblock(ToHex(block))
        if expected_error is not None and result != expected_error:
            raise AssertionError('mining the block should have failed with error %s, but submitblock returned %s' % (expected_error, result))
        elif expected_error is None and result is not None:
            raise AssertionError('submitblock returned %s' % (result))

    def mine_double_spend(self, node, txins, target_address, use_mnmerkleroot_from_tip=False):
        amount = Decimal(0)
        for txin in txins:
            txout = node.gettxout(txin['txid'], txin['vout'], False)
            amount += txout['value']
        amount -= Decimal("0.001") # fee

        rawtx = node.createrawtransaction(txins, {target_address: amount})
        rawtx = node.signrawtransaction(rawtx)['hex']
        tx = FromHex(CTransaction(), rawtx)

        self.mine_block(node, [tx], use_mnmerkleroot_from_tip=use_mnmerkleroot_from_tip)

    def test_invalid_mn_payment(self, node):
        mn_payee = self.nodes[0].getnewaddress()
        self.mine_block(node, mn_payee=mn_payee, expected_error='bad-cb-payee')
        self.mine_block(node, mn_amount=1, expected_error='bad-cb-payee')

if __name__ == '__main__':
    DIP3Test().main()
