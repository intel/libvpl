/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <chrono>
#include <cinttypes>

#include "vpl/mfx.h"

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

typedef std::chrono::high_resolution_clock HRC;
static uint64_t time_point_diff(HRC::time_point &s, HRC::time_point &e) {
    std::chrono::microseconds diff = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    return diff.count();
}

typedef struct key_name_pair_s {
    int key;
    const char *name;
} key_name_pair_t;

static const key_name_pair_t implPairs[] = {
    { MFX_IMPL_HARDWARE_ANY, "MFX_IMPL_HARDWARE_ANY" },
    { MFX_IMPL_HARDWARE, "MFX_IMPL_HARDWARE" },
    { MFX_IMPL_HARDWARE2, "MFX_IMPL_HARDWARE2" },
    { MFX_IMPL_HARDWARE3, "MFX_IMPL_HARDWARE3" },
    { MFX_IMPL_HARDWARE4, "MFX_IMPL_HARDWARE4" },
};

static const key_name_pair_t viaPairs[] = {
    { MFX_IMPL_VIA_ANY, "MFX_IMPL_VIA_ANY" },
    { MFX_IMPL_VIA_D3D9, "MFX_IMPL_VIA_D3D9" },
    { MFX_IMPL_VIA_D3D11, "MFX_IMPL_VIA_D3D11" },
};

int main(int argc, char *argv[]) {
    mfxStatus sts  = MFX_ERR_NONE;
    mfxVersion ver = { 1, 0 };
    for (size_t i = 0; i < ARRAY_SIZE(implPairs); ++i) {
        const key_name_pair_t *implPair = &implPairs[i];
        for (size_t j = 0; j < ARRAY_SIZE(viaPairs); ++j) {
            const key_name_pair_t *viaPair = &viaPairs[j];

            mfxSession session;
            HRC::time_point startTime = HRC::now();
            sts                       = MFXInit(implPair->key | viaPair->key, &ver, &session);
            HRC::time_point endTime   = HRC::now();

            uint64_t time_diff = time_point_diff(startTime, endTime);
            fprintf(stdout,
                    "mfxinit-timing  -- %25s | %-25s sts %3d  -- cost %" PRIu64 " ms\n",
                    implPair->name,
                    viaPair->name,
                    sts,
                    time_diff / 1000);

            if (session) {
                MFXClose(session);
            }
        }
    }

    printf("Finished\n");

    return 0;
}
