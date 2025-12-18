/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#define UNUSED_PARAM(x) (void)(x)

#include <stdio.h>
int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);

    printf("I'm not supposed to be executed!\n");
    return 0;
}
