/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __STRING_DEFS_H__
#define __STRING_DEFS_H__

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#ifdef __cplusplus
    #include <string>
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define msdk_stricmp              _stricmp
    #define msdk_strnlen(str, lenmax) strnlen_s(str, lenmax)
    #define msdk_strncopy_s           strncpy_s

    #define MSDK_MEMCPY_BITSTREAM(bitstream, offset, src, count) \
        memcpy_s((bitstream).Data + (offset), (bitstream).MaxLength - (offset), (src), (count))

    #define MSDK_MEMCPY_BUF(bufptr, offset, maxsize, src, count) \
        memcpy_s((bufptr) + (offset), (maxsize) - (offset), (src), (count))

    #define MSDK_MEMCPY_VAR(dstVarName, src, count) \
        memcpy_s(&(dstVarName), sizeof(dstVarName), (src), (count))

    #define MSDK_MEMCPY(dst, src, count) memcpy_s(dst, (count), (src), (count))

#else // #if defined(_WIN32) || defined(_WIN64)
    #define msdk_stricmp              strcasecmp
    #define msdk_strnlen(str, maxlen) strlen(str)

    #define msdk_strncopy_s(dst, num_dst, src, count) strncpy(dst, src, count)

    #define MSDK_MEMCPY_BITSTREAM(bitstream, offset, src, count) \
        memcpy((bitstream).Data + (offset), (src), (count))

    #define MSDK_MEMCPY_BUF(bufptr, offset, maxsize, src, count) \
        memcpy((bufptr) + (offset), (src), (count))

    #define MSDK_MEMCPY_VAR(dstVarName, src, count) memcpy(&(dstVarName), (src), (count))

    #define MSDK_MEMCPY(dst, src, count) memcpy(dst, (src), (count))

#endif // #if defined(_WIN32) || defined(_WIN64)

#ifdef __cplusplus
inline bool msdk_match(const std::string& left, const std::string& right) {
    return left == std::string(right.begin(), right.end());
}
inline bool msdk_starts_with(const std::string& left, const std::string& right) {
    return left.find(std::string(right.begin(), right.end())) == 0;
}
inline bool msdk_contains(const std::string& left, const std::string& right) {
    return left.find(std::string(right.begin(), right.end())) != left.npos;
}
#endif

#endif //__STRING_DEFS_H__
