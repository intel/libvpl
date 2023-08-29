/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __STRING_DEFS_H__
#define __STRING_DEFS_H__

#ifndef __STDC_WANT_LIB_EXT1__
    #define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <cctype>
#include <cstdarg>
#ifdef __cplusplus
    #include <string>
#endif

#ifndef __STDC_LIB_EXT1__
inline int strncpy_s(char* dest, size_t destsz, const char* src, size_t count) {
    if (!src || !dest || destsz == 0 || (count >= destsz && destsz <= strnlen(src, count))) {
        return 1;
    }
    strncpy(dest, src, count);
    return 0;
}
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define msdk_strncopy_s strncpy_s

    #define MSDK_MEMCPY_BITSTREAM(bitstream, offset, src, count) \
        memcpy_s((bitstream).Data + (offset), (bitstream).MaxLength - (offset), (src), (count))

    #define MSDK_MEMCPY_BUF(bufptr, offset, maxsize, src, count) \
        memcpy_s((bufptr) + (offset), (maxsize) - (offset), (src), (count))

    #define MSDK_MEMCPY_VAR(dstVarName, src, count) \
        memcpy_s(&(dstVarName), sizeof(dstVarName), (src), (count))

    #define MSDK_MEMCPY(dst, dest_sz, src, src_sz) memcpy_s((dst), (dest_sz), (src), (src_sz))

#else // #if defined(_WIN32) || defined(_WIN64)
    #define msdk_strncopy_s strncpy_s

    #define MSDK_MEMCPY_BITSTREAM(bitstream, offset, src, count) \
        memcpy((bitstream).Data + (offset), (src), (count))

    #define MSDK_MEMCPY_BUF(bufptr, offset, maxsize, src, count) \
        memcpy((bufptr) + (offset), (src), (count))

    #define MSDK_MEMCPY_VAR(dstVarName, src, count) memcpy(&(dstVarName), (src), (count))

    #define MSDK_MEMCPY(dst, dest_sz, src, src_sz) memcpy((dst), (src), (src_sz))

#endif // #if defined(_WIN32) || defined(_WIN64)

#ifdef __cplusplus
inline bool msdk_match(const std::string& left, const std::string& right) {
    return left == std::string(right.begin(), right.end());
}

inline bool msdk_match_ch_i(char a, char b) {
    return std::tolower(a) == std::tolower(b);
}
inline bool msdk_match_i(const std::string& left, const std::string& right) {
    if (left.length() != right.length()) {
        return false;
    }
    return std::equal(left.begin(), left.end(), right.begin(), msdk_match_ch_i);
}

inline bool msdk_starts_with(const std::string& left, const std::string& right) {
    return left.find(std::string(right.begin(), right.end())) == 0;
}
inline bool msdk_contains(const std::string& left, const std::string& right) {
    return left.find(std::string(right.begin(), right.end())) != left.npos;
}
#endif

#endif //__STRING_DEFS_H__
