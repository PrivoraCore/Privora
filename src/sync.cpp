// Copyright (c) 2011-2016 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sync.h"

#include <map>
#include <mutex>
#include <set>
#include <system_error>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "util.h"
#include "utilstrencodings.h"

#include <stdio.h>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#ifdef DEBUG_LOCKCONTENTION
void PrintLockContention(const char* pszName, const char* pszFile, int nLine)
{
    LogPrintf("LOCKCONTENTION: %s\n", pszName);
    LogPrintf("Locker: %s:%d\n", pszFile, nLine);
}
#endif /* DEBUG_LOCKCONTENTION */

#ifdef DEBUG_LOCKORDER
//
// Early deadlock detection.
// Problem being solved:
//    Thread 1 locks  A, then B, then C
//    Thread 2 locks  D, then C, then A
//     --> may result in deadlock between the two threads, depending on when they run.
// Solution implemented here:
// Keep track of pairs of locks: (A before B), (A before C), etc.
// Complain if any thread tries to lock in a different order.
//

struct CLockLocation {
    CLockLocation(const char* pszName, const char* pszFile, int nLine, bool fTryIn)
    {
        mutexName = pszName;
        sourceFile = pszFile;
        sourceLine = nLine;
        fTry = fTryIn;
    }

    std::string ToString() const
    {
        return mutexName + "  " + sourceFile + ":" + itostr(sourceLine) + (fTry ? " (TRY)" : "");
    }

    std::string MutexName() const { return mutexName; }

    bool fTry;
private:
    std::string mutexName;
    std::string sourceFile;
    int sourceLine;
};

typedef std::vector<std::pair<void*, CLockLocation> > LockStack;
typedef std::map<std::pair<void*, void*>, LockStack> LockOrders;
typedef std::set<std::pair<void*, void*> > InvLockOrders;

struct LockData {
    // Very ugly hack: as the global constructs and destructors run single
    // threaded, we use this boolean to know whether LockData still exists,
    // as DeleteLock can get called by global CCriticalSection destructors
    // after LockData disappears.
    bool available;
    LockData() : available(true) {}
    ~LockData() { available = false; }

    LockOrders lockorders;
    InvLockOrders invlockorders;
    boost::mutex dd_mutex;
} static lockdata;

boost::thread_specific_ptr<LockStack> lockstack;

static void potential_deadlock_detected(const std::pair<void*, void*>& mismatch, const LockStack& s1, const LockStack& s2)
{
    LogPrintf("POTENTIAL DEADLOCK DETECTED\n");
    LogPrintf("Previous lock order was:\n");
    BOOST_FOREACH (const PAIRTYPE(void*, CLockLocation) & i, s2) {
        if (i.first == mismatch.first) {
            LogPrintf(" (1)");
        }
        if (i.first == mismatch.second) {
            LogPrintf(" (2)");
        }
        LogPrintf(" %s\n", i.second.ToString());
    }
    LogPrintf("Current lock order is:\n");
    BOOST_FOREACH (const PAIRTYPE(void*, CLockLocation) & i, s1) {
        if (i.first == mismatch.first) {
            LogPrintf(" (1)");
        }
        if (i.first == mismatch.second) {
            LogPrintf(" (2)");
        }
        LogPrintf(" %s\n", i.second.ToString());
    }
    assert(false);
}

static void push_lock(void* c, const CLockLocation& locklocation, bool fTry)
{
    if (lockstack.get() == NULL)
        lockstack.reset(new LockStack);

    boost::unique_lock<boost::mutex> lock(lockdata.dd_mutex);

    (*lockstack).push_back(std::make_pair(c, locklocation));

    BOOST_FOREACH (const PAIRTYPE(void*, CLockLocation) & i, (*lockstack)) {
        if (i.first == c)
            break;

        std::pair<void*, void*> p1 = std::make_pair(i.first, c);
        if (lockdata.lockorders.count(p1))
            continue;
        lockdata.lockorders[p1] = (*lockstack);

        std::pair<void*, void*> p2 = std::make_pair(c, i.first);
        lockdata.invlockorders.insert(p2);
        if (lockdata.lockorders.count(p2))
            potential_deadlock_detected(p1, lockdata.lockorders[p2], lockdata.lockorders[p1]);
    }
}

static void pop_lock()
{
    (*lockstack).pop_back();
}

void EnterCritical(const char* pszName, const char* pszFile, int nLine, void* cs, bool fTry)
{
    push_lock(cs, CLockLocation(pszName, pszFile, nLine, fTry), fTry);
}

void LeaveCritical()
{
    pop_lock();
}

std::string LocksHeld()
{
    std::string result;
    BOOST_FOREACH (const PAIRTYPE(void*, CLockLocation) & i, *lockstack)
        result += i.second.ToString() + std::string("\n");
    return result;
}

void AssertLockHeldInternal(const char* pszName, const char* pszFile, int nLine, void* cs)
{
    BOOST_FOREACH (const PAIRTYPE(void*, CLockLocation) & i, *lockstack)
        if (i.first == cs)
            return;
    fprintf(stderr, "Assertion failed: lock %s not held in %s:%i; locks held:\n%s", pszName, pszFile, nLine, LocksHeld().c_str());
    abort();
}

void AssertLockNotHeldInternal(const char* pszName, const char* pszFile, int nLine, void* cs)
{
    for (const std::pair<void*, CLockLocation>& i : *lockstack) {
        if (i.first == cs) {
            fprintf(stderr, "Assertion failed: lock %s held in %s:%i; locks held:\n%s", pszName, pszFile, nLine, LocksHeld().c_str());
            abort();
        }
    }
}

void DeleteLock(void* cs)
{
    if (!lockdata.available) {
        // We're already shutting down.
        return;
    }
    boost::unique_lock<boost::mutex> lock(lockdata.dd_mutex);
    std::pair<void*, void*> item = std::make_pair(cs, (void*)0);
    LockOrders::iterator it = lockdata.lockorders.lower_bound(item);
    while (it != lockdata.lockorders.end() && it->first.first == cs) {
        std::pair<void*, void*> invitem = std::make_pair(it->first.second, it->first.first);
        lockdata.invlockorders.erase(invitem);
        lockdata.lockorders.erase(it++);
    }
    InvLockOrders::iterator invit = lockdata.invlockorders.lower_bound(item);
    while (invit != lockdata.invlockorders.end() && invit->first == cs) {
        std::pair<void*, void*> invinvitem = std::make_pair(invit->second, invit->first);
        lockdata.lockorders.erase(invinvitem);
        lockdata.invlockorders.erase(invit++);
    }
}

#endif /* DEBUG_LOCKORDER */
