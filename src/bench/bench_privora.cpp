// Copyright (c) 2015-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench.h"

#include "key.h"
#include "stacktraces.h"
#include "validation.h"
#include "util.h"

int
main(int argc, char** argv)
{
#ifdef ENABLE_CRASH_HOOKS
    RegisterPrettySignalHandlers();
    RegisterPrettyTerminateHander();
#endif
    ECC_Start();
    SetupEnvironment();
    fPrintToDebugLog = false; // don't want to write to debug.log file

    benchmark::BenchRunner::RunAll();

    ECC_Stop();
}
