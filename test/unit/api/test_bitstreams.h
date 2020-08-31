/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef TEST_UNIT_API_TEST_BITSTREAMS_H_
#define TEST_UNIT_API_TEST_BITSTREAMS_H_

class test_bitstream_96x64_8bit_hevc {
public:
    static unsigned int getlen() {
        return len;
    }
    static unsigned int getpos(unsigned int i) {
        if (i < npkt) {
            return pkt_pos[i];
        }
        else {
            return 0;
        }
    }
    static unsigned char *getdata() {
        return data;
    }

private:
    static unsigned char data[];
    static unsigned int len;
    static unsigned int pkt_pos[];
    static unsigned int npkt;
};

class test_bitstream_96x64_10bit_hevc {
public:
    static unsigned int getlen() {
        return len;
    }
    static unsigned int getpos(unsigned int i) {
        if (i < npkt) {
            return pkt_pos[i];
        }
        else {
            return 0;
        }
    }
    static unsigned char *getdata() {
        return data;
    }

private:
    static unsigned char data[];
    static unsigned int len;
    static unsigned int pkt_pos[];
    static unsigned int npkt;
};

class test_bitstream_32x32_mjpeg {
public:
    static unsigned int getlen() {
        return len;
    }
    static unsigned int getpos(unsigned int i) {
        if (i < npkt) {
            return pkt_pos[i];
        }
        else {
            return 0;
        }
    }
    static unsigned char *getdata() {
        return data;
    }

private:
    static unsigned char data[];
    static unsigned int len;
    static unsigned int pkt_pos[];
    static unsigned int npkt;
};

#endif //TEST_UNIT_API_TEST_BITSTREAMS_H_
