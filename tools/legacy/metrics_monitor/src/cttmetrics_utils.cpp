/*############################################################################
  # Copyright (C) 2018 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "cttmetrics_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char CARD_N[16] = { 0 };
char CARD[32]   = { 0 };

int path_gen(char* pdst, size_t sz, const char delim, const char* a, const char* b, const char* c) {
    size_t total_len = 2 + strlen(a) + strlen(b) + strlen(c);

    if (total_len >= sz)
        return -1;

    while (*a)
        *pdst++ = *a++;
    *pdst++ = delim;
    while (*b)
        *pdst++ = *b++;
    *pdst++ = delim;
    while (*c)
        *pdst++ = *c++;
    *pdst++ = 0;

    return 0;
}

// get the string value of property and converts it to integer
static long int get_id_from_file(const char* d_name, const char* property) {
    long int id     = 0;
    char str[16]    = { 0 };
    char fname[300] = { 0 };

    if ((NULL == d_name) || (NULL == property))
        return -1;

    if (path_gen(fname, sizeof(fname) / sizeof(fname[0]), '/', DRM_DIR, d_name, property))
        return -1;

    FILE* file = fopen(fname, "r");
    if (file) {
        if (fgets(str, sizeof(str), file)) {
            id = strtol(str, NULL, 16);
        }
        fclose(file);
    }

    return id;
}

/*
    check that system has at least one Intel grraphics adapter
    support only first adapter
*/
cttStatus discover_path_to_gpu() {
    cttStatus status  = CTT_ERR_DRIVER_NOT_FOUND;
    long int class_id = 0, vendor_id = 0;
    struct stat buffer;
    char DRM_CARD[30] = { 0 };
    int i;
    for (i = 0; i < 100; i++) {
        snprintf(CARD_N, sizeof(CARD_N), "%d", i);
        snprintf(CARD, sizeof(CARD), "card%s", CARD_N);

        path_gen(DRM_CARD, sizeof(DRM_CARD) / sizeof(DRM_CARD[0]), '/', DRM_DIR, CARD, "");
        if (lstat(DRM_CARD, &buffer))
            break;

        // device class id
        class_id = get_id_from_file(CARD, "device/class");

        if (PCI_DISPLAY_CONTROLLER_CLASS == (class_id >> 16)) {
            // device vendor id
            vendor_id = get_id_from_file(CARD, "device/vendor");
            if (vendor_id == INTEL_VENDOR_ID) {
                status = CTT_ERR_NONE;
                break;
            }
        }
    }

    return status;
}

int read_freq(int fd) {
    size_t nread;
    char freq[16]; /* Intel i915 frequencies are in MHZ, so we expect to read 4 digits */

    if (-1 == fd)
        goto error;

    /* jump to the very beginning of the file */
    if (-1 == lseek(fd, 0, SEEK_SET))
        goto error;

    /* read the new frequency value */
    nread = read(fd, freq, sizeof(freq));
    if (nread >= sizeof(freq))
        goto error;

    return atoi(freq);

error:
    return 0;
}
