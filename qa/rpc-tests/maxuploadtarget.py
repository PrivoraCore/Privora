#!/usr/bin/env python3
# Copyright (c) 2015-2016 The Privora Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.mininode import *
from test_framework.test_framework import PrivoraTestFramework
from test_framework.util import *
import time

'''
Test behavior of -maxuploadtarget.

* Verify that getdata requests for old blocks (>1week) are dropped
if uploadtarget has been reached.
* Verify that getdata requests for recent blocks are respecteved even
if uploadtarget has been reached.
* Verify that the upload counters are reset after 24 hours.
'''

# TestNode: bare-bones "peer".  Used mostly as a conduit for a test to sending
# p2p messages to a node, generating the messages in the main testing logic.
class TestNode(NodeConnCB):
    def __init__(self):
        NodeConnCB.__init__(self)
        self.connection = None
        self.ping_counter = 1
        self.last_pong = msg_pong()
        self.block_receive_map = {}

    def add_connection(self, conn):
        self.connection = conn
        self.peer_disconnected = False

    def on_inv(self, conn, message):
        pass

    # Track the last getdata message we receive (used in the test)
    def on_getdata(self, conn, message):
        self.last_getdata = message

    def on_block(self, conn, message):
        message.block.calc_sha256()
        try:
            self.block_receive_map[message.block.sha256] += 1
        except KeyError as e:
            self.block_receive_map[message.block.sha256] = 1

    # Spin until verack message is received from the node.
    # We use this to signal that our test can begin. This
    # is called from the testing thread, so it needs to acquire
    # the global lock.
    def wait_for_verack(self):
        def veracked():
            return self.verack_received
        return wait_until(veracked, timeout=10)

    def wait_for_disconnect(self):
        def disconnected():
            return self.peer_disconnected
        return wait_until(disconnected, timeout=10)

    # Wrapper for the NodeConn's send_message function
    def send_message(self, message):
        self.connection.send_message(message)

    def on_pong(self, conn, message):
        self.last_pong = message

    def on_close(self, conn):
        self.peer_disconnected = True

    # Sync up with the node after delivery of a block
    def sync_with_ping(self, timeout=30):
        def received_pong():
            return (self.last_pong.nonce == self.ping_counter)
        self.connection.send_message(msg_ping(nonce=self.ping_counter))
        success = wait_until(received_pong, timeout=timeout)
        self.ping_counter += 1
        return success

class MaxUploadTest(PrivoraTestFramework):
 
    def add_options(self, parser):
        parser.add_option("--testbinary", dest="testbinary",
                          default=os.getenv("PRIVORAD", "privorad"),
                          help="privorad binary to test")

    def __init__(self):
        super().__init__()
        self.setup_clean_chain = True
        self.num_nodes = 1

        # Cache for utxos, as the listunspent may take a long time later in the test
        self.utxo_cache = []

    def setup_network(self):
        # Start a node with maxuploadtarget of 200 MB (/24h)
        self.nodes = []
        self.nodes.append(start_node(0, self.options.tmpdir, ["-debug", "-maxuploadtarget=800", "-blockmaxsize=999000"]))

    def run_test(self):
        # Before we connect anything, we first set the time on the node
        # to be in the past, otherwise things break because the CNode
        # time counters can't be reset backward after initialization
        old_time = int(time.time() - 2*60*60*24*7)
        self.nodes[0].setmocktime(old_time)

        # Generate some old blocks
        self.nodes[0].generate(130)

        # test_nodes[0] will only request old blocks
        # test_nodes[1] will only request new blocks
        # test_nodes[2] will test resetting the counters
        test_nodes = []
        connections = []

        for i in range(3):
            test_nodes.append(TestNode())
            connections.append(NodeConn('127.0.0.1', p2p_port(0), self.nodes[0], test_nodes[i]))
            test_nodes[i].add_connection(connections[i])

        NetworkThread().start() # Start up network handling in another thread
        [x.wait_for_verack() for x in test_nodes]

        # Test logic begins here

        # Now mine a big block
        mine_large_block(self.nodes[0], self.utxo_cache)

        # Store the hash; we'll request this later
        big_old_block = self.nodes[0].getbestblockhash()
        old_block_size = self.nodes[0].getblock(big_old_block, True)['size']
        big_old_block = int(big_old_block, 16)

        # Advance to two days ago
        self.nodes[0].setmocktime(int(time.time()) - 2*60*60*24)

        # Mine one more block, so that the prior block looks old
        mine_large_block(self.nodes[0], self.utxo_cache)

        # We'll be requesting this new block too
        big_new_block = self.nodes[0].getbestblockhash()
        big_new_block = int(big_new_block, 16)

        # test_nodes[0] will test what happens if we just keep requesting the
        # the same big old block too many times (expect: disconnect)

        getdata_request = msg_getdata()
        getdata_request.inv.append(CInv(2, big_old_block))

        max_bytes_per_day = 800*1024*1024
        daily_buffer = 144 * 4000000
        max_bytes_available = max_bytes_per_day - daily_buffer
        success_count = max_bytes_available // old_block_size

        # 576MB will be reserved for relaying new blocks, so expect this to
        # succeed for ~235 tries.
        for i in range(success_count):
            test_nodes[0].send_message(getdata_request)
            test_nodes[0].sync_with_ping()
            assert_equal(test_nodes[0].block_receive_map[big_old_block], i+1)

        assert_equal(len(self.nodes[0].getpeerinfo()), 3)
        # At most a couple more tries should succeed (depending on how long 
        # the test has been running so far).
        for i in range(3):
            test_nodes[0].send_message(getdata_request)
        test_nodes[0].wait_for_disconnect()
        assert_equal(len(self.nodes[0].getpeerinfo()), 2)
        print("Peer 0 disconnected after downloading old block too many times")

        # Requesting the current block on test_nodes[1] should succeed indefinitely,
        # even when over the max upload target.
        # We'll try 800 times
        getdata_request.inv = [CInv(2, big_new_block)]
        for i in range(800):
            test_nodes[1].send_message(getdata_request)
            test_nodes[1].sync_with_ping()
            assert_equal(test_nodes[1].block_receive_map[big_new_block], i+1)

        print("Peer 1 able to repeatedly download new block")

        # But if test_nodes[1] tries for an old block, it gets disconnected too.
        getdata_request.inv = [CInv(2, big_old_block)]
        test_nodes[1].send_message(getdata_request)
        test_nodes[1].wait_for_disconnect()
        assert_equal(len(self.nodes[0].getpeerinfo()), 1)

        print("Peer 1 disconnected after trying to download old block")

        print("Advancing system time on node to clear counters...")

        # If we advance the time by 24 hours, then the counters should reset,
        # and test_nodes[2] should be able to retrieve the old block.
        self.nodes[0].setmocktime(int(time.time()))
        test_nodes[2].sync_with_ping()
        test_nodes[2].send_message(getdata_request)
        test_nodes[2].sync_with_ping()
        assert_equal(test_nodes[2].block_receive_map[big_old_block], 1)

        print("Peer 2 able to download old block")

        [c.disconnect_node() for c in connections]

        #stop and start node 0 with 1MB maxuploadtarget, whitelist 127.0.0.1
        print("Restarting nodes with -whitelist=127.0.0.1")
        stop_node(self.nodes[0], 0)
        self.nodes[0] = start_node(0, self.options.tmpdir, ["-debug", "-whitelist=127.0.0.1", "-maxuploadtarget=1", "-blockmaxsize=999000"])

        #recreate/reconnect 3 test nodes
        test_nodes = []
        connections = []

        for i in range(3):
            test_nodes.append(TestNode())
            connections.append(NodeConn('127.0.0.1', p2p_port(0), self.nodes[0], test_nodes[i]))
            test_nodes[i].add_connection(connections[i])

        NetworkThread().start() # Start up network handling in another thread
        [x.wait_for_verack() for x in test_nodes]

        #retrieve 20 blocks which should be enough to break the 1MB limit
        getdata_request.inv = [CInv(2, big_new_block)]
        for i in range(20):
            test_nodes[1].send_message(getdata_request)
            test_nodes[1].sync_with_ping()
            assert_equal(test_nodes[1].block_receive_map[big_new_block], i+1)

        getdata_request.inv = [CInv(2, big_old_block)]
        test_nodes[1].send_message(getdata_request)
        test_nodes[1].wait_for_disconnect()
        assert_equal(len(self.nodes[0].getpeerinfo()), 3) #node is still connected because of the whitelist

        print("Peer 1 still connected after trying to download old block (whitelisted)")

        [c.disconnect_node() for c in connections]

if __name__ == '__main__':
    MaxUploadTest().main()
