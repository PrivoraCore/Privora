#!/usr/bin/env python3
# Copyright (c) 2014-2016 The Privora Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# Base class for RPC testing

import logging
import optparse
import os
import sys
import shutil
import tempfile
import traceback
import unittest
from concurrent.futures import ThreadPoolExecutor
from time import time, sleep
from .mininode import wait_until

from .util import (
    assert_equal,
    initialize_chain,
    start_node,
    start_nodes,
    connect_nodes_bi,
    connect_nodes,
    sync_blocks,
    sync_mempools,
    sync_znodes,
    stop_nodes,
    stop_node,
    start_node,
    enable_coverage,
    check_json_precision,
    initialize_chain_clean,
    PortSeed,
    p2p_port,
    wait_to_sync_znodes,
    satoshi_round,
    wait_to_sync,
    copy_datadir,
    set_mocktime,
    get_mocktime,
    set_node_times,
    enable_mocktime
)
from .authproxy import JSONRPCException


class PrivoraTestFramework(object):

    def __init__(self):
        self.num_nodes = 4
        self.setup_clean_chain = False
        self.nodes = None
        self.set_test_params()

    # Methods to override in subclass test scripts.
    def set_test_params(self):
        """Tests must implement this method to change default values for number of nodes, topology, etc"""
        pass #raise NotImplementedError, do not raise an exception, as not all tests implement this.

    def run_test(self):
        raise NotImplementedError

    def add_options(self, parser):
        pass

    def setup_chain(self):
        self.log.info("Initializing test directory "+self.options.tmpdir)
        if self.setup_clean_chain:
            initialize_chain_clean(self.options.tmpdir, self.num_nodes)
        else:
            initialize_chain(self.options.tmpdir, self.num_nodes, self.options.cachedir)

    def stop_node(self, num_node):
        stop_node(self.nodes[num_node], num_node)

    def setup_nodes(self):
        return start_nodes(self.num_nodes, self.options.tmpdir)

    def setup_network(self, split = False):
        self.nodes = self.setup_nodes()

        # Connect the nodes as a "chain".  This allows us
        # to split the network between nodes 1 and 2 to get
        # two halves that can work on competing chains.

        # If we joined network halves, connect the nodes from the joint
        # on outward.  This ensures that chains are properly reorganised.

        if self.num_nodes > 1:
            if not split:
                connect_nodes_bi(self.nodes, 1, 2)
                sync_blocks(self.nodes[1:3])
                sync_mempools(self.nodes[1:3])

            connect_nodes_bi(self.nodes, 0, 1)
            connect_nodes_bi(self.nodes, 2, 3)
        self.is_network_split = split
        self.sync_all()

    def split_network(self):
        """
        Split the network of four nodes into nodes 0/1 and 2/3.
        """
        assert not self.is_network_split
        stop_nodes(self.nodes)
        self.setup_network(True)

    def sync_all(self):
        if self.is_network_split:
            sync_blocks(self.nodes[:2])
            sync_blocks(self.nodes[2:])
            sync_mempools(self.nodes[:2])
            sync_mempools(self.nodes[2:])
        else:
            sync_blocks(self.nodes)
            sync_mempools(self.nodes)

    def znsync_all(self):
        if self.is_network_split:
            sync_znodes(self.nodes[:2])
            sync_znodes(self.nodes[2:])
        else:
            sync_znodes(self.nodes)

    def join_network(self):
        """
        Join the (previously split) network halves together.
        """
        assert self.is_network_split
        stop_nodes(self.nodes)
        self.setup_network(False)

    def main(self):

        parser = optparse.OptionParser(usage="%prog [options]")
        parser.add_option("--nocleanup", dest="nocleanup", default=False, action="store_true",
                          help="Leave privorads and test.* datadir on exit or error")
        parser.add_option("--noshutdown", dest="noshutdown", default=False, action="store_true",
                          help="Don't stop privorads after the test execution")
        parser.add_option("--srcdir", dest="srcdir", default=os.path.normpath(os.path.dirname(os.path.realpath(__file__))+"/../../../src"),
                          help="Source directory containing privorad/privora-cli (default: %default)")
        parser.add_option("--cachedir", dest="cachedir", default="",
                          help="Directory for caching pregenerated datadirs")
        parser.add_option("--tmpdir", dest="tmpdir", default=tempfile.mkdtemp(prefix="test"),
                          help="Root directory for datadirs")
        parser.add_option("-l", "--loglevel", dest="loglevel", default="INFO",
                          help="log events at this level and higher to the console. Can be set to DEBUG, INFO, WARNING, ERROR or CRITICAL. Passing --loglevel DEBUG will output all logs to console. Note that logs at all levels are always written to the test_framework.log file in the temporary test directory.")
        parser.add_option("--tracerpc", dest="trace_rpc", default=False, action="store_true",
                          help="Print out all RPC calls as they are made")
        parser.add_option("--portseed", dest="port_seed", default=os.getpid(), type='int',
                          help="The seed to use for assigning port numbers (default: current process id)")
        parser.add_option("--coveragedir", dest="coveragedir",
                          help="Write tested RPC commands into this directory")
        self.add_options(parser)
        (self.options, self.args) = parser.parse_args()

        # backup dir variable for removal at cleanup
        self.options.root, self.options.tmpdir = self.options.tmpdir, self.options.tmpdir + '/' + str(self.options.port_seed)

        if self.options.cachedir == "":
            self.options.cachedir = self.options.tmpdir

        if self.options.trace_rpc:
            logging.basicConfig(level=logging.DEBUG, stream=sys.stdout)

        if self.options.coveragedir:
            enable_coverage(self.options.coveragedir)

        PortSeed.n = self.options.port_seed

        os.environ['PATH'] = self.options.srcdir+":"+self.options.srcdir+"/qt:"+os.environ['PATH']

        check_json_precision()

        # Set up temp directory and start logging
        os.makedirs(self.options.tmpdir, exist_ok=False)
        self._start_logging()
        success = False
        try:
            self.setup_chain()
            self.setup_network()
            self.run_test()
            success = True
        except JSONRPCException as e:
            self.log.exception("JSONRPC error")
        except AssertionError as e:
            self.log.exception("Assertion failed")
        except KeyError as e:
            self.log.exception("Key error")
        except Exception as e:
            self.log.exception("Unexpected exception caught during testing")
        except KeyboardInterrupt as e:
            self.log.warning("Exiting after keyboard interrupt")

        if not self.options.noshutdown:
            self.log.info("Stopping nodes")
            try:
                stop_nodes(self.nodes)
            except BaseException as e:
                success = False
                print("Unexpected exception caught during shutdown: " + repr(e))
                traceback.print_tb(sys.exc_info()[2])
        else:
            self.log.info("Note: dashds were not stopped and may still be running")

        if not self.options.nocleanup and not self.options.noshutdown and success:
            self.log.info("Cleaning up")
            shutil.rmtree(self.options.tmpdir)
            if not os.listdir(self.options.root):
                os.rmdir(self.options.root)
        else:
            self.log.warning("Not cleaning up dir %s" % self.options.tmpdir)
            if os.getenv("PYTHON_DEBUG", ""):
                # Dump the end of the debug logs, to aid in debugging rare
                # travis failures.
                import glob
                filenames = glob.glob(self.options.tmpdir + "/node*/regtest/debug.log")
                MAX_LINES_TO_PRINT = 1000
                for f in filenames:
                    print("From" , f, ":")
                    from collections import deque
                    print("".join(deque(open(f), MAX_LINES_TO_PRINT)))
        if success:
            self.log.info("Tests successful")
            sys.exit(0)
        else:
            self.log.error("Test failed. Test logging available at %s/test_framework.log", self.options.tmpdir)
            logging.shutdown()
            sys.exit(1)

    def _start_logging(self):
        # Add logger and logging handlers
        self.log = logging.getLogger('TestFramework')
        self.log.setLevel(logging.DEBUG)
        # Create file handler to log all messages
        fh = logging.FileHandler(self.options.tmpdir + '/test_framework.log')
        fh.setLevel(logging.DEBUG)
        # Create console handler to log messages to stderr. By default this logs only error messages, but can be configured with --loglevel.
        ch = logging.StreamHandler(sys.stdout)
        # User can provide log level as a number or string (eg DEBUG). loglevel was caught as a string, so try to convert it to an int
        ll = int(self.options.loglevel) if self.options.loglevel.isdigit() else self.options.loglevel.upper()
        ch.setLevel(ll)
        # Format logs the same as privorad's debug.log with microprecision (so log files can be concatenated and sorted)
        formatter = logging.Formatter(fmt = '%(asctime)s.%(msecs)03d000 %(name)s (%(levelname)s): %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
        fh.setFormatter(formatter)
        ch.setFormatter(formatter)
        # add the handlers to the logger
        self.log.addHandler(fh)
        self.log.addHandler(ch)

        if self.options.trace_rpc:
            rpc_logger = logging.getLogger("PrivoraRPC")
            rpc_logger.setLevel(logging.DEBUG)
            rpc_handler = logging.StreamHandler(sys.stdout)
            rpc_handler.setLevel(logging.DEBUG)
            rpc_logger.addHandler(rpc_handler)
# Test framework for doing p2p comparison testing, which sets up some privorad
# binaries:
# 1 binary: test binary
# 2 binaries: 1 test binary, 1 ref binary
# n>2 binaries: 1 test binary, n-1 ref binaries

class ComparisonTestFramework(PrivoraTestFramework):

    def __init__(self):
        super().__init__()
        self.num_nodes = 2
        self.setup_clean_chain = True

    def add_options(self, parser):
        parser.add_option("--testbinary", dest="testbinary",
                          default=os.getenv("PRIVORAD", "privorad"),
                          help="privorad binary to test")
        parser.add_option("--refbinary", dest="refbinary",
                          default=os.getenv("PRIVORAD", "privorad"),
                          help="privorad binary to use for reference nodes (if any)")

    def setup_network(self):
        self.nodes = start_nodes(
            self.num_nodes, self.options.tmpdir,
            extra_args=[['-debug', '-whitelist=127.0.0.1']] * self.num_nodes,
            binary=[self.options.testbinary] +
            [self.options.refbinary]*(self.num_nodes-1))


#
# Znode tests support
#

ZNODE_COLLATERAL = 1000

class ZnodeCollateral(object):
    def __init__(self):
        self.tx_id = None
        self.n = -1

    def __str__(self):
        return self.tx_id + ": " + str(self.n)

    def parse_collateral_output(self, target_address, tx_text, tx_id):
        for vout in tx_text["vout"]:
            if vout["value"] == ZNODE_COLLATERAL and vout["scriptPubKey"]["addresses"] == [target_address]:
                self.tx_id = tx_id
                self.n = vout["n"]
        return self

class ZnodeTestFramework(PrivoraTestFramework):
    def __init__(self):
        super().__init__()
        self.znode_priv_keys = dict()
        self.num_nodes = 4
        self.num_znodes = 3

    def setup_chain(self):
        self.log.info("Initializing test directory "+self.options.tmpdir)
        self.setup_clean_chain = True
        super().setup_chain()

    def setup_network(self, split=False):
        super().setup_network(split)
        for i in range(self.num_nodes):
            for j in range(i, self.num_nodes):
                connect_nodes_bi(self.nodes, i, j)

    def write_master_znode_conf(self, znode, collateral):
        znode_service = get_znode_service(znode)
        znode_conf = " ".join(["zn"+str(znode), znode_service, self.znode_priv_keys[znode], collateral.tx_id, str(collateral.n)])

        master_node_conf_filename = os.path.join(self.options.tmpdir, "node" + str(znode), "regtest", "znode.conf")
        with open(master_node_conf_filename, "a") as master_node_conf:
            master_node_conf.write(znode_conf)
            master_node_conf.write("\n")
            master_node_conf.close()

    def generate_znode_collateral(self, generator_node=None):
        if generator_node is None:
            generator_node = self.num_nodes - 1
        curr_balance = self.nodes[generator_node].getbalance()
        while curr_balance < ZNODE_COLLATERAL:
            self.nodes[generator_node].generate(int((ZNODE_COLLATERAL - curr_balance) / 25))
            curr_balance = self.nodes[generator_node].getbalance()
        return curr_balance

    def send_znode_collateral(self, znode, collateral_provider=None):
        if collateral_provider is None:
            collateral_provider = self.num_nodes - 1
        znode_address = self.nodes[znode].getaccountaddress("Znode")
        tx_id = self.nodes[collateral_provider].sendtoaddress(znode_address, ZNODE_COLLATERAL)
        tx_text = self.nodes[collateral_provider].getrawtransaction(tx_id, 1)
        collateral = ZnodeCollateral()
        return collateral.parse_collateral_output(znode_address, tx_text, tx_id)

    def send_mature_znode_collateral(self, znode, collateral_provider=None):
        if collateral_provider is None:
            collateral_provider = self.num_nodes - 1
        result = self.send_znode_collateral(znode, collateral_provider)
        self.nodes[collateral_provider].generate(10)
        sync_blocks(self.nodes)
        time.sleep(3)
        return result

    def configure_znode(self, znode, master_znode=None):
        if master_znode is None:
            master_znode = self.num_nodes - 1
        self.znode_priv_keys[znode] = self.nodes[znode].znode("genkey")
        stop_node(self.nodes[znode], znode)
        znode_service = get_znode_service(znode)
        self.nodes[znode] = start_node(znode, self.options.tmpdir, ["-znode", "-znodeprivkey="+self.znode_priv_keys[znode], "-externalip="+znode_service, "-listen"])
        connect_nodes(self.nodes[znode], znode)
        for i in range(self.num_nodes):
            if i != znode:
                connect_nodes_bi(self.nodes, i, znode)

    def generate_znode_privkey(self, znode):
        self.znode_priv_keys[znode] = self.nodes[znode].znode("genkey")

    def restart_as_znode(self, znode):
        stop_node(self.nodes[znode], znode)
        znode_service = get_znode_service(znode)
        self.nodes[znode] = start_node(znode, self.options.tmpdir, ["-znode", "-znodeprivkey="+self.znode_priv_keys[znode], "-externalip="+znode_service, "-listen"])
        connect_nodes(self.nodes[znode], znode)
        for i in range(self.num_nodes):
            if i != znode:
                connect_nodes_bi(self.nodes, i, znode)
        for i in range(self.num_nodes):
            wait_to_sync_znodes(self.nodes[i])

    def znode_start(self, znode):
        assert_equal("Znode successfully started", self.nodes[znode].znode("start"))

    def configure_znode(self, znode, master_znode=None ):
        self.generate_znode_privkey(znode, master_znode)
        self.restart_as_znode(znode)

    def wait_znode_enabled(self, enabled_znode_number, znode_to_wait_on = None, timeout = 10):
        if znode_to_wait_on is None:
            znode_to_wait_on = self.num_nodes - 1
        wait_to_sync_znodes(self.nodes[znode_to_wait_on])
        for j in range (timeout):
            if self.nodes[znode_to_wait_on].znode("count", "enabled") == enabled_znode_number:
                return
            time.sleep(1)
        raise Exception("Cannot wait until znodes enabled")

    def generate(self, blocks, generator_node=None):
        if generator_node is None:
            generator_node = self.num_nodes - 1
        for b in range(blocks):
            self.nodes[generator_node].generate(1)
            sync_blocks(self.nodes)
            time.sleep(1)


def get_znode_service(znode):
    znode_ip_str = "127.0.1." + str(znode + 1)
    znode_port_str = str(p2p_port(znode))
    return znode_ip_str + ":" + znode_port_str

class ZnodeInfo:
    def __init__(self, proTxHash, ownerAddr, votingAddr, pubKeyOperator, keyOperator, collateral_address, collateral_txid, collateral_vout):
        self.proTxHash = proTxHash
        self.ownerAddr = ownerAddr
        self.votingAddr = votingAddr
        self.pubKeyOperator = pubKeyOperator
        self.keyOperator = keyOperator
        self.collateral_address = collateral_address
        self.collateral_txid = collateral_txid
        self.collateral_vout = collateral_vout

class EvoZnodeTestFramework(PrivoraTestFramework):
    def __init__(self, num_nodes, masterodes_count, extra_args=None):
        super().__init__()
        self.mn_count = masterodes_count
        self.num_nodes = num_nodes
        self.mninfo = []
        self.setup_clean_chain = True
        self.is_network_split = False
        # additional args
        if extra_args is None:
            extra_args = [[]] * num_nodes
        assert_equal(len(extra_args), num_nodes)
        self.extra_args = extra_args

    def create_simple_node(self):
        idx = len(self.nodes)
        args = self.extra_args[idx]
        self.nodes.append(start_node(idx, self.options.tmpdir, args))
        for i in range(0, idx):
            connect_nodes(self.nodes[i], idx)

    def prepare_masternodes(self):
        for idx in range(0, self.mn_count):
            self.prepare_masternode(idx)

    def prepare_masternode(self, idx):
        bls = self.nodes[0].bls('generate')
        address = self.nodes[0].getnewaddress()
        txid = self.nodes[0].sendtoaddress(address, ZNODE_COLLATERAL)

        txraw = self.nodes[0].getrawtransaction(txid, True)
        collateral_vout = 0
        for vout_idx in range(0, len(txraw["vout"])):
            vout = txraw["vout"][vout_idx]
            if vout["value"] == ZNODE_COLLATERAL:
                collateral_vout = vout_idx
        self.nodes[0].lockunspent(False, [{'txid': txid, 'vout': collateral_vout}])

        # send to same address to reserve some funds for fees
        self.nodes[0].sendtoaddress(address, 0.001)

        ownerAddr = self.nodes[0].getnewaddress()
        votingAddr = self.nodes[0].getnewaddress()
        rewardsAddr = self.nodes[0].getnewaddress()

        port = p2p_port(len(self.nodes) + idx)
        if (idx % 2) == 0:
            self.nodes[0].lockunspent(True, [{'txid': txid, 'vout': collateral_vout}])
            proTxHash = self.nodes[0].protx('register_fund', address, '127.0.0.1:%d' % port, ownerAddr, bls['public'], votingAddr, 0, rewardsAddr, address)
        else:
            self.nodes[0].generate(1)
            proTxHash = self.nodes[0].protx('register', txid, collateral_vout, '127.0.0.1:%d' % port, ownerAddr, bls['public'], votingAddr, 0, rewardsAddr, address)
        self.nodes[0].generate(1)

        self.mninfo.append(ZnodeInfo(proTxHash, ownerAddr, votingAddr, bls['public'], bls['secret'], address, txid, collateral_vout))
        self.sync_all()

    def remove_mastermode(self, idx):
        mn = self.mninfo[idx]
        rawtx = self.nodes[0].createrawtransaction([{"txid": mn.collateral_txid, "vout": mn.collateral_vout}], {self.nodes[0].getnewaddress(): 999.9999})
        rawtx = self.nodes[0].signrawtransaction(rawtx)
        self.nodes[0].sendrawtransaction(rawtx["hex"])
        self.nodes[0].generate(1)
        self.sync_all()
        self.mninfo.remove(mn)

    def prepare_datadirs(self):
        # stop faucet node so that we can copy the datadir
        stop_node(self.nodes[0], 0)

        start_idx = len(self.nodes)
        for idx in range(0, self.mn_count):
            copy_datadir(0, idx + start_idx, self.options.tmpdir)

        # restart faucet node
        self.nodes[0] = start_node(0, self.options.tmpdir, self.extra_args[0])

    def start_masternodes(self):
        start_idx = len(self.nodes)

        for idx in range(0, self.mn_count):
            self.nodes.append(None)
        executor = ThreadPoolExecutor(max_workers=20)

        def do_start(idx):
            args = ['-znode=1',
                    '-znodeblsprivkey=%s' % self.mninfo[idx].keyOperator
                    ] + self.extra_args[idx + start_idx]
            node = start_node(idx + start_idx, self.options.tmpdir, args)
            self.mninfo[idx].nodeIdx = idx + start_idx
            self.mninfo[idx].node = node
            self.nodes[idx + start_idx] = node

        def do_connect(idx):
            # Connect to the control node only, masternodes should take care of intra-quorum connections themselves
            connect_nodes(self.mninfo[idx].node, 0)

        jobs = []

        # start up nodes in parallel
        for idx in range(0, self.mn_count):
            jobs.append(executor.submit(do_start, idx))

        # wait for all nodes to start up
        for job in jobs:
            job.result()
        jobs.clear()

        # connect nodes in parallel
        for idx in range(0, self.mn_count):
            jobs.append(executor.submit(do_connect, idx))

        # wait for all nodes to connect
        for job in jobs:
            job.result()
        jobs.clear()

        sync_blocks(self.nodes)
        self.nodes[0].generate(1)
        sync_znodes(self.nodes, True)

        executor.shutdown()

    def setup_network(self):
        self.nodes = []
        enable_mocktime()
        # create faucet node for collateral and transactions
        self.nodes.append(start_node(0, self.options.tmpdir, self.extra_args[0]))
        required_balance = ZNODE_COLLATERAL * self.mn_count + 1
        while self.nodes[0].getbalance() < required_balance:
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            self.nodes[0].generate(1)
        # create connected simple nodes
        for i in range(0, self.num_nodes - self.mn_count - 1):
            self.create_simple_node()
        sync_znodes(self.nodes, True)

        # activate DIP3
        while self.nodes[0].getblockcount() < 550:
            self.nodes[0].generate(10)
        self.sync_all()

        # create masternodes
        self.prepare_masternodes()
        self.prepare_datadirs()
        self.start_masternodes()

        # non-masternodes where disconnected from the control node during prepare_datadirs,
        # let's reconnect them back to make sure they receive updates
        num_simple_nodes = self.num_nodes - self.mn_count - 1
        for i in range(0, num_simple_nodes):
            connect_nodes(self.nodes[i+1], 0)
            wait_to_sync(self.nodes[i+1], true)

        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(1)
        # sync nodes
        self.sync_all()
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())

        mn_info = self.nodes[0].evoznodelist("status")
        assert (len(mn_info) == self.mn_count)
        for status in mn_info.values():
            assert (status == 'ENABLED')

    def get_autois_bip9_status(self, node):
        info = node.getblockchaininfo()
        # we reuse the dip3 deployment
        return info['bip9_softforks']['dip0003']['status']

    def activate_autois_bip9(self, node):
        # sync nodes periodically
        # if we sync them too often, activation takes too many time
        # if we sync them too rarely, nodes failed to update its state and
        # bip9 status is not updated
        # so, in this code nodes are synced once per 20 blocks
        counter = 0
        sync_period = 10

        while self.get_autois_bip9_status(node) == 'defined':
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            node.generate(1)
            counter += 1
            if counter % sync_period == 0:
                # sync nodes
                self.sync_all()

        while self.get_autois_bip9_status(node) == 'started':
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            node.generate(1)
            counter += 1
            if counter % sync_period == 0:
                # sync nodes
                self.sync_all()

        while self.get_autois_bip9_status(node) == 'locked_in':
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            node.generate(1)
            counter += 1
            if counter % sync_period == 0:
                # sync nodes
                self.sync_all()

        # sync nodes
        self.sync_all()

        assert(self.get_autois_bip9_status(node) == 'active')

    def get_autois_spork_state(self, node):
        info = node.spork('active')
        return info['SPORK_16_INSTANTSEND_AUTOLOCKS']

    def set_autois_spork_state(self, node, state):
        # Increment mocktime as otherwise nodes will not update sporks
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        if state:
            value = 0
        else:
            value = 4070908800
        node.spork('SPORK_16_INSTANTSEND_AUTOLOCKS', value)

    def create_raw_tx(self, node_from, node_to, amount, min_inputs, max_inputs):
        assert (min_inputs <= max_inputs)
        # fill inputs
        inputs = []
        balances = node_from.listunspent()
        in_amount = 0.0
        last_amount = 0.0
        for tx in balances:
            if len(inputs) < min_inputs:
                input = {}
                input["txid"] = tx['txid']
                input['vout'] = tx['vout']
                in_amount += float(tx['amount'])
                inputs.append(input)
            elif in_amount > amount:
                break
            elif len(inputs) < max_inputs:
                input = {}
                input["txid"] = tx['txid']
                input['vout'] = tx['vout']
                in_amount += float(tx['amount'])
                inputs.append(input)
            else:
                input = {}
                input["txid"] = tx['txid']
                input['vout'] = tx['vout']
                in_amount -= last_amount
                in_amount += float(tx['amount'])
                inputs[-1] = input
            last_amount = float(tx['amount'])

        assert (len(inputs) >= min_inputs)
        assert (len(inputs) <= max_inputs)
        assert (in_amount >= amount)
        # fill outputs
        receiver_address = node_to.getnewaddress()
        change_address = node_from.getnewaddress()
        fee = 0.001
        outputs = {}
        outputs[receiver_address] = satoshi_round(amount)
        outputs[change_address] = satoshi_round(in_amount - amount - fee)
        rawtx = node_from.createrawtransaction(inputs, outputs)
        ret = node_from.signrawtransaction(rawtx)
        decoded = node_from.decoderawtransaction(ret['hex'])
        ret = {**decoded, **ret}
        return ret

    # sends regular instantsend with high fee
    def send_regular_instantsend(self, sender, receiver, check_fee = True):
        receiver_addr = receiver.getnewaddress()
        txid = sender.instantsendtoaddress(receiver_addr, 1.0)
        if (check_fee):
            MIN_FEE = satoshi_round(-0.0001)
            fee = sender.gettransaction(txid)['fee']
            expected_fee = MIN_FEE * len(sender.getrawtransaction(txid, True)['vin'])
            assert_equal(fee, expected_fee)
        return self.wait_for_instantlock(txid, sender)

    # sends simple tx, it should become locked if autolocks are allowed
    def send_simple_tx(self, sender, receiver):
        raw_tx = self.create_raw_tx(sender, receiver, 1.0, 1, 4)
        txid = self.nodes[0].sendrawtransaction(raw_tx['hex'])
        self.sync_all()
        return self.wait_for_instantlock(txid, sender)

    # sends complex tx, it should never become locked for old instentsend
    def send_complex_tx(self, sender, receiver):
        raw_tx = self.create_raw_tx(sender, receiver, 1.0, 5, 100)
        txid = sender.sendrawtransaction(raw_tx['hex'])
        self.sync_all()
        return self.wait_for_instantlock(txid, sender)

    def wait_for_tx(self, txid, node, expected=True, timeout=15):
        def check_tx():
            try:
                return node.getrawtransaction(txid)
            except:
                return False
        w = wait_until(check_tx, timeout=timeout, sleep=0.5)
        if not w and expected:
            raise AssertionError("wait_for_instantlock failed")
        elif w and not expected:
            raise AssertionError("waiting unexpectedly succeeded")

    def wait_for_instantlock(self, txid, node, expected=True, timeout=15, do_assert=False):
        def check_instantlock():
            try:
                return node.getrawtransaction(txid, True)["instantlock"]
            except:
                return False
        w = wait_until(check_instantlock, timeout=timeout, sleep=0.1)
        if not w and expected:
            if do_assert:
                raise AssertionError("wait_for_instantlock failed")
            else:
                return False
        elif w and not expected:
            if do_assert:
                raise AssertionError("waiting unexpectedly succeeded")
            else:
                return False
        return True

    def wait_for_chainlocked_block(self, node, block_hash, expected=True, timeout=15):
        def check_chainlocked_block():
            try:
                block = node.getblock(block_hash)
                return block["confirmations"] > 0 and block["chainlock"]
            except:
                return False
        w = wait_until(check_chainlocked_block, timeout=timeout, sleep=0.1)
        if not w and expected:
            raise AssertionError("wait_for_chainlocked_block failed")
        elif w and not expected:
            raise AssertionError("waiting unexpectedly succeeded")

    def wait_for_chainlocked_block_all_nodes(self, block_hash, timeout=15):
        for node in self.nodes:
            self.wait_for_chainlocked_block(node, block_hash, timeout=timeout)

    def wait_for_sporks_same(self, timeout=30):
        st = time()
        while time() < st + timeout:
            if self.check_sporks_same():
                return
            sleep(0.5)
        raise AssertionError("wait_for_sporks_same timed out")

    def check_sporks_same(self):
        sporks = self.nodes[0].spork('show')
        for node in self.nodes[1:]:
            sporks2 = node.spork('show')
            if sporks != sporks2:
                return False
        return True

    def wait_for_quorum_phase(self, phase, check_received_messages, check_received_messages_count, timeout=30):
        t = time()
        while time() - t < timeout:
            all_ok = True
            for mn in self.mninfo:
                s = mn.node.quorum("dkgstatus")["session"]
                if "llmq_5_60" not in s:
                    all_ok = False
                    break
                s = s["llmq_5_60"]
                if "phase" not in s:
                    all_ok = False
                    break
                if s["phase"] != phase:
                    all_ok = False
                    break
                if check_received_messages is not None:
                    if s[check_received_messages] < check_received_messages_count:
                        all_ok = False
                        break
            if all_ok:
                return
            sleep(0.1)
        raise AssertionError("wait_for_quorum_phase timed out")

    def wait_for_quorum_commitment(self, timeout = 15):
        t = time()
        while time() - t < timeout:
            all_ok = True
            for node in self.nodes:
                s = node.quorum("dkgstatus")
                if "minableCommitments" not in s:
                    all_ok = False
                    break
                s = s["minableCommitments"]
                if "llmq_5_60" not in s:
                    all_ok = False
                    break
            if all_ok:
                return
            sleep(0.1)
        raise AssertionError("wait_for_quorum_commitment timed out")

    def mine_quorum(self, expected_contributions=5, expected_complaints=0, expected_justifications=0, expected_commitments=5):
        quorums = self.nodes[0].quorum("list")

        # move forward to next DKG
        skip_count = 24 - (self.nodes[0].getblockcount() % 24)
        if skip_count != 0:
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            self.nodes[0].generate(skip_count)
        sync_blocks(self.nodes)

        # Make sure all reached phase 1 (init)
        self.wait_for_quorum_phase(1, None, 0)
        # Give nodes some time to connect to neighbors
        sleep(2)
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(2)
        sync_blocks(self.nodes)

        # Make sure all reached phase 2 (contribute) and received all contributions
        self.wait_for_quorum_phase(2, "receivedContributions", expected_contributions)
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(2)
        sync_blocks(self.nodes)

        # Make sure all reached phase 3 (complain) and received all complaints
        self.wait_for_quorum_phase(3, "receivedComplaints", expected_complaints)
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(2)
        sync_blocks(self.nodes)

        # Make sure all reached phase 4 (justify)
        self.wait_for_quorum_phase(4, "receivedJustifications", expected_justifications)
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(2)
        sync_blocks(self.nodes)

        # Make sure all reached phase 5 (commit)
        self.wait_for_quorum_phase(5, "receivedPrematureCommitments", expected_commitments)
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(2)
        sync_blocks(self.nodes)

        # Make sure all reached phase 6 (mining)
        self.wait_for_quorum_phase(6, None, 0)

        # Wait for final commitment
        self.wait_for_quorum_commitment()

        # mine the final commitment
        set_mocktime(get_mocktime() + 1)
        set_node_times(self.nodes, get_mocktime())
        self.nodes[0].generate(1)
        while quorums == self.nodes[0].quorum("list"):
            sleep(2)
            set_mocktime(get_mocktime() + 1)
            set_node_times(self.nodes, get_mocktime())
            self.nodes[0].generate(1)
            sync_blocks(self.nodes)
        new_quorum = self.nodes[0].quorum("list", 1)["llmq_5_60"][0]

        # Mine 8 (SIGN_HEIGHT_OFFSET) more blocks to make sure that the new quorum gets eligable for signing sessions
        self.nodes[0].generate(8)

        sync_blocks(self.nodes)

        return new_quorum

    def wait_for_mnauth(self, node, count, timeout=10):
        def test():
            pi = node.getpeerinfo()
            c = 0
            for p in pi:
                if "verified_proregtx_hash" in p and p["verified_proregtx_hash"] != "":
                    c += 1
            return c >= count
        assert wait_until(test, timeout=timeout)

    def generate_until_sigma_activated(self, num_node):
        node = self.nodes[num_node]

        self.sync_all()
        required_block = 550
        current_block = node.getblockcount()
        if current_block >= required_block:
            return []

        return node.generate(required_block - current_block)
