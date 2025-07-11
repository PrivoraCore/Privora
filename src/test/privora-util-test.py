#!/usr/bin/env python
# Copyright 2014 BitPay Inc.
# Copyright 2016 The Privora Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
from __future__ import division,print_function,unicode_literals
import os
import bctest
import buildenv
import argparse
import logging

help_text="""Test framework for privora utils.

Runs automatically during `make check`.

Can also be run manually from the src directory by specifying the source directory:

test/privora-util-test.py --srcdir='srcdir' [--verbose]
"""

if __name__ == '__main__':
    # Try to get the source directory from the environment variables. This will
    # be set for `make check` automated runs. If environment variable is not set,
    # then get the source directory from command line args.
    try:
        srcdir = os.environ["srcdir"]
        verbose = False
    except:
        parser = argparse.ArgumentParser(description=help_text)
        parser.add_argument('-s', '--srcdir')
        parser.add_argument('-v', '--verbose', action='store_true')
        args = parser.parse_args()
        srcdir = args.srcdir
        verbose = args.verbose

    if verbose:
        level = logging.DEBUG
    else:
        level = logging.ERROR
    formatter = '%(asctime)s - %(levelname)s - %(message)s'
    # Add the format/level to the logger
    logging.basicConfig(format = formatter, level=level)

    bctest.bctester(srcdir + "/test/data", "privora-util-test.json", buildenv)
