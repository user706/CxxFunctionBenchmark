// bsls_stackaddressutil.t.cpp                                        -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#include <bsls_stackaddressutil.h>

#include <bsls_bsltestutil.h>
#include <bsls_platform.h>
#include <bsls_types.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BSLS_PLATFORM_OS_WINDOWS
// 'getStackAddresses' will not be able to trace through our stack frames if
// we're optimized on Windows

# pragma optimize("", off)

#endif

using namespace BloombergLP;

//=============================================================================
//                                  TEST PLAN
//-----------------------------------------------------------------------------
// [ 3]  int getStackAddresses(void **buffer, int maxFrames);
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [ 4] USAGE EXAMPLE
// [ 2] getStackAddresses(0, 0)
// [-1] Speed benchmark of getStackAddresses

//=============================================================================
//                  STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
// NOTE: THIS IS A LOW-LEVEL COMPONENT AND MAY NOT USE ANY C++ LIBRARY
// FUNCTIONS, INCLUDING IOSTREAMS.
static int testStatus = 0;

static void aSsErT(bool b, const char *s, int i)
{
    if (b) {
        printf("Error " __FILE__ "(%d): %s    (failed)\n", i, s);
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}

# define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                       STANDARD BDE TEST DRIVER MACROS
//-----------------------------------------------------------------------------
#define ASSERTV BSLS_BSLTESTUTIL_ASSERTV

#define Q   BSLS_BSLTESTUTIL_Q   // Quote identifier literally.
#define P   BSLS_BSLTESTUTIL_P   // Print identifier and value.
#define P_  BSLS_BSLTESTUTIL_P_  // P(X) without '\n'.
#define T_  BSLS_BSLTESTUTIL_T_  // Print a tab (w/o newline).
#define L_  BSLS_BSLTESTUTIL_L_  // current Line number

// ============================================================================
//               GLOBAL HELPER VARIABLES AND TYPES FOR TESTING
// ----------------------------------------------------------------------------

typedef bsls::Types::UintPtr UintPtr;
int verbose;
int veryVerbose;

// 'lamePlatform' -- on lame platforms where StackAddressUtil doesn't work or
// executables are stripped, disable some of the tests.
#if defined(BDE_BUILD_TARGET_OPT) && defined(BSLS_PLATFORM_OS_WINDOWS)
const bool lamePlatform = true;
#else
const bool lamePlatform = false;
#endif

// ============================================================================
//                    GLOBAL HELPER FUNCTIONS FOR TESTING
// ----------------------------------------------------------------------------

static std::string myHex(UintPtr up)
{
    std::stringstream ss;
    ss.setf(std::ios_base::hex, std::ios_base::basefield);
    ss.setf(std::ios_base::showbase);
    ss << up;
    return ss.str();
}

                                // ------
                                // CASE 4
                                // ------

namespace CASE_FOUR {

// First, we define 'AddressEntry', which will contain a pointer to the
// beginning of a function and an index corresponding to the function.  The '<'
// operator is defined so that a vector of address entries can be sorted in the
// order of the function addresses.  The address entries will be populated so
// that the entry containing '&funcN' when 'N' is an integer will have an index
// of 'N'.

struct AddressEntry {
    void *d_funcAddress;
    int   d_index;

    // CREATORS
    AddressEntry(void *funcAddress, int index)
    : d_funcAddress(funcAddress)
    , d_index(index)
        // Create an 'AddressEntry' object and initialize it with the
        // specified 'funcAddress' and 'index'.
    {}

    bool operator<(const AddressEntry& rhs) const
        // Return 'true' if the address stored in the object is lower than
        // the address stored in 'rhs' and 'false' otherwise.  Note that
        // this is a member function for brevity, it only exists to
        // facilitate sorting 'AddressEntry' objects in a vector.
    {

        return d_funcAddress < rhs.d_funcAddress;
    }
};
// Then, we define 'entries', a vector of address entries.  This will be
// populated such that a given entry will contain function address '&funcN' and
// index 'N'.  The elements will be sorted according to function address.

std::vector<AddressEntry> entries;

// Next, we define 'findIndex':

static int findIndex(const void *retAddress)
    // Return the index of the address entry whose function uses an
    // instruction located at specified 'retAddress'.  The behavior is
    // undefined unless 'retAddress' is the address of an instruction in
    // use by a function referred to by an address entry in 'entries'.
{
    unsigned int u = 0;
    while (u < entries.size()-1 && retAddress >= entries[u+1].d_funcAddress) {
        ++u;
    }
    ASSERT(u < entries.size());
    ASSERT(retAddress >= entries[u].d_funcAddress);

    int ret = entries[u].d_index;

    if (veryVerbose) {
        P_(retAddress) P_(entries[u].d_funcAddress) P(ret);
    }

    return ret;
}

// Then, we define a volatile global variable that we will use in calculation
// to discourage compiler optimizers from inlining:

volatile unsigned int volatileGlobal = 1;

// Next, we define a set of functions that will be called in a nested fashion
// -- 'func5' calls 'func4' who calls 'fun3' and so on.  In each function, we
// will perform some inconsequential instructions to prevent the compiler from
// inlining the functions.
//
// Note that we know the 'if' conditions in these 5 subroutines never evaluate
// to 'true', however, the optimizer cannot figure that out, and that will
// prevent it from inlining here.

static unsigned int func1();
static unsigned int func2()
{
    if (volatileGlobal > 10) {
        return (volatileGlobal -= 100) * 2 * func2();                 // RETURN
    }
    else {
        return volatileGlobal * 2 * func1();                          // RETURN
    }
}
static unsigned int func3()
{
    if (volatileGlobal > 10) {
        return (volatileGlobal -= 100) * 2 * func3();                 // RETURN
    }
    else {
        return volatileGlobal * 3 * func2();                          // RETURN
    }
}
static unsigned int func4()
{
    if (volatileGlobal > 10) {
        return (volatileGlobal -= 100) * 2 * func4();                 // RETURN
    }
    else {
        return volatileGlobal * 4 * func3();                          // RETURN
    }
}
static unsigned int func5()
{
    if (volatileGlobal > 10) {
        return (volatileGlobal -= 100) * 2 * func5();                 // RETURN
    }
    else {
        return volatileGlobal * 5 * func4();                          // RETURN
    }
}
static unsigned int func6()
{
    if (volatileGlobal > 10) {
        return (volatileGlobal -= 100) * 2 * func6();                 // RETURN
    }
    else {
        return volatileGlobal * 6 * func5();                          // RETURN
    }
}

// Next, we define the macro FUNC_ADDRESS, which will take a parameter of
// '&<function name>' and return a pointer to the actual beginning of the
// function's code, which is a non-trivial and platform-dependent exercise.
// Note: this doesn't work on Windows for global routines.

#if   defined(BSLS_PLATFORM_OS_HPUX)
# define FUNC_ADDRESS(p) (((void **) (void *) (p))[sizeof(void *) == 4])
#elif defined(BSLS_PLATFORM_OS_AIX)
# define FUNC_ADDRESS(p) (((void **) (void *) (p))[0])
#else
# define FUNC_ADDRESS(p) ((void *) (p))
#endif

// Then, we define 'func1', the last function to be called in the chain of
// nested function calls.  'func1' uses
// 'bsls::StackAddressUtil::getStackAddresses' to get an ordered sequence of
// return addresses from the current thread's function call stack and uses the
// previously defined 'findIndex' function to verify those address are correct.

unsigned int func1()
    // Call 'getAddresses' and verify that the returned set of addresses
    // matches our expectations.
{
    // Next, we populate and sort the 'entries' table, a sorted array of
    // 'AddressEntry' objects that will allow 'findIndex' to look up within
    // which function a given return address can be found.

    entries.clear();
    entries.push_back(AddressEntry(0, 0));
    entries.push_back(AddressEntry(FUNC_ADDRESS(&func1), 1));
    entries.push_back(AddressEntry(FUNC_ADDRESS(&func2), 2));
    entries.push_back(AddressEntry(FUNC_ADDRESS(&func3), 3));
    entries.push_back(AddressEntry(FUNC_ADDRESS(&func4), 4));
    entries.push_back(AddressEntry(FUNC_ADDRESS(&func5), 5));
    entries.push_back(AddressEntry(FUNC_ADDRESS(&func6), 6));
    std::sort(entries.begin(), entries.end());

    // Then, we obtain the stack addresses with 'getStackAddresses'.

    enum { BUFFER_LENGTH = 100 };
    void *buffer[BUFFER_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    int numAddresses = bsls::StackAddressUtil::getStackAddresses(
                                                                buffer,
                                                                BUFFER_LENGTH);
    ASSERT(numAddresses >= (int) entries.size());
    ASSERT(numAddresses < BUFFER_LENGTH);
    ASSERT(0 != buffer[numAddresses-1]);
    ASSERT(0 == buffer[numAddresses]);

    // Finally, we go through several of the first addresses returned in
    // 'buffer' and verify that each address corresponds to the routine we
    // expect it to.

    // Note that on some, but not all, platforms there is an extra 'narcissic'
    // frame describing 'getStackAddresses' itself at the beginning of
    // 'buffer'.  By starting our iteration through 'buffer' at
    // 'k_IGNORE_FRAMES', we guarantee that the first address we examine will
    // be in 'func1' on all platforms.

    int funcIdx  = 1;
    int stackIdx = bsls::StackAddressUtil::k_IGNORE_FRAMES;
    for (; funcIdx < (int) entries.size(); ++funcIdx, ++stackIdx) {
        ASSERT(stackIdx < numAddresses);
        ASSERT(funcIdx == findIndex(buffer[stackIdx]));
    }

    if (testStatus || veryVerbose) {
        Q(Entries:);
        for (unsigned int u = 0; u < entries.size(); ++u) {
            P_(u); P_((void *) entries[u].d_funcAddress);
            P(entries[u].d_index);
        }

        Q(Stack:);
        for (int i = 0; i < numAddresses; ++i) {
            P_(i); P(buffer[i]);
        }
    }

    return volatileGlobal;
}

#undef FUNC_ADDRESS

}  // close namespace CASE_FOUR

                                // ------
                                // CASE 3
                                // ------

namespace CASE_THREE {

struct AddressEntry {
    UintPtr d_returnAddress;
    int     d_traceIndex;
};

bool operator<(const AddressEntry lhs, const AddressEntry rhs)
{
    return lhs.d_returnAddress < rhs.d_returnAddress;
}

static int findIndex(AddressEntry *entries, int numAddresses, UintPtr funcP)
    // 'buffer' is a sorted list of 'numAddresses' addresses, whose highest
    // entry is the max possible value for a pointer.  Search the buffer and
    // return the index of the first entry at or before 'p'.
{
    int i = 0;
    while (i < numAddresses && funcP > entries[i].d_returnAddress) {
        ++i;
    }
    ASSERT(i < numAddresses);
    UintPtr retP = entries[i].d_returnAddress;

    ASSERT(retP > funcP);
    int ret = entries[i].d_traceIndex;

    if (veryVerbose) {
        P_(myHex(funcP).c_str()) P_(myHex(retP).c_str()) P(ret);
    }

    return ret;
}

#define CASE3_FUNC(nMinus1, n)                                                \
    void func ## n(int *pi)                                                   \
    {                                                                         \
        ++*pi;                                                                \
        if (*pi > 100) {                                                      \
            func ## n(pi);                                                    \
        }                                                                     \
        else if (*pi < 100) {                                                 \
            func ## nMinus1(pi);                                              \
        }                                                                     \
                                                                              \
        ++*pi;                                                                \
    }

void func0(int *pi);
CASE3_FUNC(0, 1)
CASE3_FUNC(1, 2)
CASE3_FUNC(2, 3)
CASE3_FUNC(3, 4)
CASE3_FUNC(4, 5)

#if    defined(BSLS_PLATFORM_OS_HPUX) && defined(BSLS_PLATFORM_CPU_32_BIT)
# define FUNC_ADDRESS_NUM(p) (((UintPtr *) (UintPtr) (p))[1])
#elif (defined(BSLS_PLATFORM_OS_HPUX) && defined(BSLS_PLATFORM_CPU_64_BIT)) \
    || defined(BSLS_PLATFORM_OS_AIX)
# define FUNC_ADDRESS_NUM(p) (((UintPtr *) (UintPtr) (p))[0])
#else
# define FUNC_ADDRESS_NUM(p) ((UintPtr) (p))
#endif

void func0(int *pi)
{
    enum { BUFFER_LENGTH = 100,
           IGNORE_FRAMES = bsls::StackAddressUtil::k_IGNORE_FRAMES
    };

    *pi += 2;

    void *buffer[BUFFER_LENGTH];
    AddressEntry entries[BUFFER_LENGTH];

    UintPtr funcAddrs[] = { FUNC_ADDRESS_NUM(&func0),
                            FUNC_ADDRESS_NUM(&func1),
                            FUNC_ADDRESS_NUM(&func2),
                            FUNC_ADDRESS_NUM(&func3),
                            FUNC_ADDRESS_NUM(&func4),
                            FUNC_ADDRESS_NUM(&func5) };
    enum { NUM_FUNC_ADDRS = sizeof funcAddrs / sizeof *funcAddrs };

    memset(buffer, 0, sizeof(buffer));
    int numAddresses = bsls::StackAddressUtil::getStackAddresses(
                                                                buffer,
                                                                BUFFER_LENGTH);

    for (int toIndex = 0, fromIndex = IGNORE_FRAMES;
                            fromIndex < numAddresses; ++toIndex, ++fromIndex) {
        entries[toIndex].d_returnAddress = (UintPtr) buffer[fromIndex];
        entries[toIndex].d_traceIndex    = toIndex;
    }
    numAddresses -= IGNORE_FRAMES;

    std::sort(entries, entries + numAddresses);

    for (int i = 0; i < numAddresses - 1; ++i) {
        const AddressEntry *e = &entries[i];

        UintPtr lhs = e[0].d_returnAddress;
        UintPtr rhs = e[1].d_returnAddress;
        ASSERTV(i, lhs, rhs, lhs < rhs);
    }

    bool problem = false;
    for (int i = 0; i < NUM_FUNC_ADDRS; ++i) {
        int index = findIndex(entries, numAddresses, funcAddrs[i]);
        if (i != index) {
            problem = true;
        }
        ASSERTV(i, index, myHex(funcAddrs[i]).c_str(), i == index);
    }

    if (problem || veryVerbose) {
        for (int i = 0; i < NUM_FUNC_ADDRS; ++i) {
            P_(i);    P(myHex(funcAddrs[i]).c_str());
        }

        for (int i = 0; i < numAddresses; ++i) {
            const AddressEntry *e = &entries[i];

            printf("(%d): addr = %s, ti = %d\n", i,
                   myHex(e->d_returnAddress).c_str(), e->d_traceIndex);
        }
    }
}

}  // close namespace CASE_THREE

                                // ------
                                // Case 1
                                // ------

namespace CASE_ONE {

volatile int recurseDepth = 50;

enum {
    BUFFER_LENGTH = 1000
};

void recurser(volatile int *depth)
{

    if (--*depth > 0) {
        recurser(depth);
    }
    else {
        void *buffer[BUFFER_LENGTH];
        int numAddresses;

        memset(buffer, 0, sizeof(buffer));
        numAddresses = bsls::StackAddressUtil::getStackAddresses(
                                                                buffer,
                                                                BUFFER_LENGTH);
        ASSERTV(numAddresses, lamePlatform || numAddresses > recurseDepth);
        for (int i = 0; i < numAddresses; ++i) {
            ASSERT(0 != buffer[i]);
        }
        for (int i = numAddresses; i < BUFFER_LENGTH; ++i) {
            ASSERT(0 == buffer[i]);
        }

        memset(buffer, 0, sizeof(buffer));
        numAddresses = bsls::StackAddressUtil::getStackAddresses(buffer, 10);
        ASSERTV(numAddresses, lamePlatform || 10 == numAddresses);
        for (int i = 0; i < numAddresses; ++i) {
            ASSERT(0 != buffer[i]);
        }
        for (int i = numAddresses; i < BUFFER_LENGTH; ++i) {
            ASSERT(0 == buffer[i]);
        }
    }

    ++*depth;           // Prevent compilers from optimizing tail recursion as
                        // a loop.
}

}  // close namespace CASE_ONE

                            // ------------------
                            // case -1: benchmark
                            // ------------------

namespace CASE_MINUS_ONE {

typedef int (*GetStackPointersFunc)(void **buffer,
                                    int    maxFrames);
GetStackPointersFunc funcPtr;

enum CustomValues {
    // This enum has a name to placate the IBM C++03 compiler which requires
    // that all template parameters to be named types.
    RECURSION_DEPTH = 40,
    MAX_FRAMES = 100
};

void recurser(int  iterations,
              int *depth)
{
    if (--*depth <= 0) {
        void *addresses[MAX_FRAMES];
        for (int i = iterations; i > 0; --i) {
            int frames = (*funcPtr)(addresses, MAX_FRAMES);
            ASSERTV(RECURSION_DEPTH, frames, RECURSION_DEPTH < frames);
            ASSERTV(RECURSION_DEPTH + 10, frames,
                                                RECURSION_DEPTH + 10 > frames);
        }
    }
    else {
        recurser(iterations, depth);
    }

    ++*depth;         // prevent tail recursion optimization
}

}  // close namespace CASE_MINUS_ONE

// ============================================================================
//                               MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    verbose  = argc > 2;
    veryVerbose = argc > 3 ? (atoi(argv[3]) ? atoi(argv[3]) : 1) : 0;

    printf("TEST " __FILE__ " CASE %d\n", test);

    switch (test) { case 0:
      case 4: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //
        // Concerns:
        //: 1 The usage example provided in the component header file compiles,
        //:   links, and runs as shown.
        //
        // Plan:
        //: 1 Have a sequence of records, each of which contains the address of
        //:   the beginning of a function's code and an index corresponding to
        //:   that function.
        //:
        //: 2 These functions are arranged in a chain to call each other and
        //:   the final one of the chain calls 'getStackAddresses' and looks up
        //:   the return addresses obtain in the sequence of records to verify
        //:   that the addresses are within the functions we expect them to be
        //:   in.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) printf("Finding Right Functions Test\n"
                            "============================\n");

#ifndef BSLS_PLATFORM_OS_WINDOWS
        // This test case just seems to fail on Windows, something to do with
        // '&' not working correctly, possibly because the compiler is creating
        // 'thunk' functions which just call the actual routine.  I wish they
        // wouldn't do that.

        unsigned int result = CASE_FOUR::func6();
        ASSERTV(result, 6 * 5 * 4 * 3 * 2, result == 6 * 5 * 4 * 3 * 2);
#endif
      }  break;
      case 3: {
        // --------------------------------------------------------------------
        // CLASS METHOD 'getStackAddresses'
        //
        // Note: this was the original test that 'getStackAddresses' was
        // finding the proper functions.  This test was later simplified into
        // case 4 for use as a usage example.
        //
        // Concerns:
        //: 1 The method finds the functions we expect it to.
        //:
        //: 2 The returned addresses are in the right order.
        //:
        //: 3 The method returns non-zero on error.
        //:
        //: 4 QoI: Asserted precondition violations are detected when enabled.
        //
        // Plan:
        //: 1 Have a sequence of functions on the stack, and take pointers to
        //:   those functions using '&<function name>', put those pointers into
        //:   an array.
        //:
        //: 2 Collect the return addresses from the stack and put those
        //:   addresses into an array of records, also storing an int in each
        //:   record to identify which routine we expect the address to be in.
        //:   Sort the array of records.  It then becomes possible to verify
        //:   which return address is within which routine.
        //
        // Testing:
        //   int getStackAddresses(void **buffer, int maxFrames);
        // --------------------------------------------------------------------

        if (verbose) printf("Finding Right Functions Test\n"
                            "============================\n");

#ifndef BSLS_PLATFORM_OS_WINDOWS
        // This test case just seems to fail on Windows, something to do with
        // '&' not working correctly, possibly because the compiler is creating
        // 'thunk' functions which just call the actual routine.  I wish they
        // wouldn't do that.

        int i = 0;
        CASE_THREE::func5(&i);
        ASSERTV(i, 12 == i);
#endif
      }  break;
      case 2: {
        // --------------------------------------------------------------------
        // ZEROES TEST CASE
        //
        // Concerns:
        //: 1 'getStackAddresses(0, 0)' doesn't segFault.
        //
        // Plan:
        //: 1 Call 'getStackAddresses(0, 0)'.  In the debugger, verify that on
        //:   Linux, the first call calls 'backtrace' and the second call calls
        //:   neither 'dlopen' nor 'malloc'.
        //
        // Testing:
        //   CONCERN: 'getStackAddresses(0, 0)' doesn't segFault.
        // --------------------------------------------------------------------

        if (verbose) printf("getStackAddresses(0, 0) TEST\n"
                            "============================\n");

        bsls::StackAddressUtil::getStackAddresses(0, 0);
        bsls::StackAddressUtil::getStackAddresses(0, 0);
      }  break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //   This case exercises (but does not fully test) basic functionality.
        //
        // Concerns:
        //: 1 The class is sufficiently functional to enable comprehensive
        //:   testing in subsequent test cases.
        //:
        //: 2 The method won't write past the end of the array it is passed.
        //
        // Plan:
        //: 1 Recurse many times, call 'getStackAddresses', and check that the
        //:   elements are either null or non-null as expected.
        //:
        //: 2 Do this twice, once in the case where the array is more than long
        //:   enough to accomodate the entire stack depth, and once in the case
        //:   where the array length passed is too short to hold the entire
        //:   stack, and verify that elements past the specified length of the
        //:   array are unaffected.
        //
        // Testing:
        //   BREATHING TEST
        // --------------------------------------------------------------------

        if (verbose) printf("BREATHING TEST\n"
                            "==============\n");

        namespace TC = CASE_ONE;

        // Call 'recurseAndPrintExample3' with will recurse 'depth' times, then
        // print a stack trace.

        veryVerbose = std::max(0, veryVerbose);
        TC::recurseDepth += veryVerbose;
        int depth = TC::recurseDepth;
        TC::recurser(&depth);
        ASSERT(TC::recurseDepth == depth);
      }  break;
      default: {
        fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        fprintf(stderr, "Error, non-zero test status = %d.\n", testStatus);
    }

    return testStatus;
}

// ----------------------------------------------------------------------------
// Copyright 2015 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
