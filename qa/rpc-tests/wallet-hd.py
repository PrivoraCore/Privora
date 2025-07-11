#!/usr/bin/env python3
# Copyright (c) 2016 The Privora Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import PrivoraTestFramework
from test_framework.util import (
    start_nodes,
    start_node,
    assert_equal,
    connect_nodes_bi,
)
import os
import shutil
from test_framework.test_helper import get_dumpwallet_otp 


class WalletHDTest(PrivoraTestFramework):

    def __init__(self):
        super().__init__()
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.node_args = [['-usehd=0'], ['-usehd=1', '-keypool=0']]

    def setup_network(self):
        self.nodes = start_nodes(self.num_nodes, self.options.tmpdir, self.node_args)
        self.is_network_split = False
        connect_nodes_bi(self.nodes, 0, 1)

    def run_test (self):
        tmpdir = self.options.tmpdir

        # Make sure we use hd, keep masterkeyid
        masterkeyid = self.nodes[1].getwalletinfo()['hdmasterkeyid']
        assert_equal(len(masterkeyid), 40)

        # Import a non-HD private key in the HD wallet
        non_hd_add = self.nodes[0].getnewaddress()

        key = None
        try:
            self.nodes[1].importprivkey(self.nodes[0].dumpprivkey(non_hd_add))
        except Exception as ex:
            key = get_dumpwallet_otp (ex.error['message'])
            self.nodes[1].importprivkey(self.nodes[0].dumpprivkey(non_hd_add, key))
        assert key, 'Import wallet did not raise exception when was called first time without one-time code.'
        

        # This should be enough to keep the master key and the non-HD key 
        self.nodes[1].backupwallet(tmpdir + "/hd.bak")
        #self.nodes[1].dumpwallet(tmpdir + "/hd.dump")

        # Derive some HD addresses and remember the last
        # Also send funds to each add
        self.nodes[0].generate(101)
        hd_add = None
        num_hd_adds = 300
        for i in range(num_hd_adds):
            hd_add = self.nodes[1].getnewaddress()
            hd_info = self.nodes[1].validateaddress(hd_add)
            assert_equal(hd_info["hdkeypath"], "m/44'/1'/0'/0/" + str(i+1))
            assert_equal(hd_info["hdmasterkeyid"], masterkeyid)
            self.nodes[0].sendtoaddress(hd_add, 1)
            self.nodes[0].generate(1)
        self.nodes[0].sendtoaddress(non_hd_add, 1)
        self.nodes[0].generate(1)

        self.sync_all()
        assert_equal(self.nodes[1].getbalance(), num_hd_adds + 1)

        print("Restore backup ...")
        self.stop_node(1)
        # we need to delete the complete regtest directory
        # otherwise node1 would auto-recover all funds in flag the keypool keys as used
        shutil.rmtree(tmpdir + "/node1/regtest/blocks")
        shutil.rmtree(tmpdir + "/node1/regtest/chainstate")
        shutil.copyfile(tmpdir + "/hd.bak", tmpdir + "/node1/regtest/wallet.dat")
        self.nodes[1] = start_node(1, self.options.tmpdir, self.node_args[1])

        # Assert that derivation is deterministic
        hd_add_2 = None
        for _ in range(num_hd_adds):
            hd_add_2 = self.nodes[1].getnewaddress()
            hd_info_2 = self.nodes[1].validateaddress(hd_add_2)
            assert_equal(hd_info_2["hdkeypath"], "m/44'/1'/0'/0/" + str(_+1))
            assert_equal(hd_info_2["hdmasterkeyid"], masterkeyid)
        assert_equal(hd_add, hd_add_2)
        connect_nodes_bi(self.nodes, 0, 1)
        self.sync_all()

        # Needs rescan
        self.stop_node(1)
        self.nodes[1] = start_node(1, self.options.tmpdir, self.node_args[1] + ['-rescan'])
        #connect_nodes_bi(self.nodes, 0, 1)
        assert_equal(self.nodes[1].getbalance(), num_hd_adds + 1)


if __name__ == '__main__':
    WalletHDTest().main ()
