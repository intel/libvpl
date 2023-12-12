//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include <iostream>
#include <thread>
#include "./decode.h"
#include "./encode.h"
#include "./util.h"
#include "./vpp.h"

mfxStatus RunTest(int tIndex, CTest *test, Options *pOpts, const std::string &testType) {
    mfxStatus sts = test->Init(tIndex, pOpts);
    if (sts == MFX_ERR_NONE) {
        sts = test->Run();
        if (sts != MFX_ERR_NONE) {
            printf("ERROR: could not run %s test\n", testType.c_str());
        }
    }

    return sts;
}

template <typename T>
mfxStatus ValSurfaceSharingAPI(Options opts) {
    mfxStatus sts = MFX_ERR_NONE;

    if (opts.bTestMultiSession == true) {
        // if there's only 1 adapter, test 2 threads for multi-session testing.
        // add the 2nd adapter number same as 1st one.
        if (opts.adapterNumbers.size() == 1)
            opts.adapterNumbers.push_back(opts.adapterNumbers[0]);

        int numAdapters = (int)opts.adapterNumbers.size();

        std::vector<std::thread> threads;
        std::vector<T *> testInstances;
        std::vector<Options> tOpts(numAdapters, opts);

        printf("\nCreate %d threads for the processing\n", numAdapters);
        for (int i = 0; i < numAdapters; i++) {
            tOpts[i].adapterToRun = opts.adapterNumbers[i];
            tOpts[i].outfileName =
                opts.outfileName + "." + std::to_string(i + 1); // ex: out.h265.1, 2, ..
            printf("  %d. adapter number: %d, output file name: %s\n",
                   i + 1,
                   opts.adapterNumbers[i],
                   tOpts[i].outfileName.c_str());

            T *test = new T();
            threads.push_back(std::thread(RunTest, i, test, &tOpts[i], typeid(T).name()));
            testInstances.push_back(test);
        }
        printf("\n");

        for (auto &t : threads) {
            t.join();
        }

        for (auto &ti : testInstances) {
            delete ti;
        }
    }
    else {
        T *test = new T;
        sts     = RunTest(0, test, &opts, typeid(T).name());
        delete test;
    }

    return sts;
}

int main(int argc, char **argv) {
    mfxStatus sts = MFX_ERR_NONE;
    Options opts  = {};

    //-- Parse command line args to opts
    if (ParseArgsAndValidate(argc, argv, &opts) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    if (opts.bShowAdaptersInfo == true) {
        return GetAdaptersInfo(&opts, true);
    }

    if (opts.bTestMultiSession == true) {
        sts = GetAdaptersInfo(&opts);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    switch (opts.surfaceComponent) {
        case MFX_SURFACE_COMPONENT_ENCODE:
            sts = ValSurfaceSharingAPI<CEncodeTest>(opts);
            break;
        case MFX_SURFACE_COMPONENT_DECODE:
            sts = ValSurfaceSharingAPI<CDecodeTest>(opts);
            break;
        case MFX_SURFACE_COMPONENT_VPP_INPUT:
        case MFX_SURFACE_COMPONENT_VPP_OUTPUT:
            sts = ValSurfaceSharingAPI<CVPPTest>(opts);
            break;
        default:
            break;
    }

    return sts;
}
