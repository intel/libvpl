/*############################################################################
  # Copyright (C) 2021 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
  
#include "va/va.h"

/* These macro required for code compilation. */

/* end of internal stuff */

void appnd_e1(VADisplay va_display, int width, int height) {
/*beg1*/
const int num_surfaces = 5;
VASurfaceID surfaces[num_surfaces];
VASurfaceAttrib attrib;

attrib.type = VASurfaceAttribPixelFormat;
attrib.value.type = VAGenericValueTypeInteger;
attrib.value.value.i = VA_FOURCC_NV12;
attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;

vaCreateSurfaces(va_display, VA_RT_FORMAT_YUV420, width, height,
                 surfaces, num_surfaces, &attrib, 1);
/*end1*/

/*beg2*/
vaDestroySurfaces(va_display, surfaces, num_surfaces);
/*end2*/
}


void appnd_e2(VADisplay va_display, VASurfaceID surfaceToMap) {
/*beg3*/
VAImage image;
unsigned char *Y, *U, *V;
void* buffer;

vaDeriveImage(va_display, surfaceToMap, &image);
vaMapBuffer(va_display, image.buf, &buffer);

/* NV12 */
Y = (unsigned char*)buffer + image.offsets[0];
U = (unsigned char*)buffer + image.offsets[1];
V = U + 1;
/*end3*/
Y = Y;
V = V;
/*beg4*/
vaUnmapBuffer(va_display, image.buf);
vaDestroyImage(va_display, image.image_id);
/*end4*/
}

void appnd_e3(VADisplay va_display, VAContextID va_context, unsigned int buf_size) {
/*beg5*/
VABufferID buf_id;
size_t size;
uint32_t offset;
void *buf;

/* create buffer */
vaCreateBuffer(va_display, va_context, VAEncCodedBufferType, buf_size, 1, NULL, & buf_id);

/* encode frame */
// ...

/* map buffer */
VACodedBufferSegment *coded_buffer_segment;

vaMapBuffer(va_display, buf_id, (void **)(&coded_buffer_segment));

size   = coded_buffer_segment->size;
offset = coded_buffer_segment->bit_offset;
buf    = coded_buffer_segment->buf;

/* retrieve encoded data*/
// ...

/* unmap and destroy buffer */
vaUnmapBuffer(va_display, buf_id);
vaDestroyBuffer(va_display, buf_id);
/*end5*/

size = size;
offset = offset;
buf = buf;
}
