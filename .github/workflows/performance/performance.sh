#!/bin/bash
if [ ! -f content/hevc_1920x1080p_bits.h265 ]; then
    mkdir content
    ffmpeg -y -hide_banner \
        -hwaccel_output_format qsv \
        -f lavfi -i testsrc2=size=1920x1080:rate=30:duration=100.0 \
        -threads 1 \
        -c:v hevc_qsv -vf format=pix_fmts=nv12 \
        -f hevc content/hevc_1920x1080p_bits.h265
fi
. /package/etc/vpl/vars.sh
measure perf -e 1 content/hevc_1920x1080p_bits.h265
PERF_DATA=data/artifacts/measure/perf
mkdir -p /results/${PERF_DATA}
cp -rv /opt/${PERF_DATA}/msperf* /results/${PERF_DATA}/
