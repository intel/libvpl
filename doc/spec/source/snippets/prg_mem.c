#include <stdlib.h>

#include "mfxdefs.h"
#include "mfxstructures.h"
#include "mfxsession.h"
#include "mfxvideo.h"

/* These macro required for code compilation. */
#define UNUSED_PARAM(x) (void)(x)

mfxSession session;
mfxStatus sts;
mfxFrameSurface1 *outsurface;
mfxSyncPoint syncp;
mfxHDL device_handle, resource;
mfxHandleType device_type;
mfxResourceType resource_type;

static int isDeviceTypeCompatible(mfxHandleType device_type)
{
    UNUSED_PARAM(device_type);
    return 1;
}

static int isPossibleForMemorySharing(mfxHDL device_handle)
{
    UNUSED_PARAM(device_handle);
    return 1;
}

static int isResourceTypeCompatible(mfxResourceType resource_type)
{
    UNUSED_PARAM(resource_type);
    return 1;
}

static void ProcessNativeMemory(mfxHDL hdl)
{
    UNUSED_PARAM(hdl);
    return;
}

static void ProcessSystemMemory(mfxFrameSurface1 *surf)
{
    UNUSED_PARAM(surf);
    return;
}
/* end of internal stuff */


/*beg1*/
#define ALIGN32(X) (((mfxU32)((X)+31)) & (~ (mfxU32)31))

typedef struct {
   mfxU16 width, height;
   mfxU8 *base;
} mid_struct;

mfxStatus fa_alloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response) {
   UNUSED_PARAM(pthis);
   if (!(request->Type&MFX_MEMTYPE_SYSTEM_MEMORY))
      return MFX_ERR_UNSUPPORTED;
   if (request->Info.FourCC!=MFX_FOURCC_NV12)
      return MFX_ERR_UNSUPPORTED;
   response->NumFrameActual=request->NumFrameMin;
   for (int i=0;i<request->NumFrameMin;i++) {
      mid_struct *mmid=(mid_struct *)malloc(sizeof(mid_struct));
      mmid->width=ALIGN32(request->Info.Width);
      mmid->height=ALIGN32(request->Info.Height);
      mmid->base=(mfxU8*)malloc(mmid->width*mmid->height*3/2);
      response->mids[i]=mmid;
   }
   return MFX_ERR_NONE;
}

mfxStatus fa_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) {
   UNUSED_PARAM(pthis);
   mid_struct *mmid=(mid_struct *)mid;
   ptr->Pitch=mmid->width;
   ptr->Y=mmid->base;
   ptr->U=ptr->Y+mmid->width*mmid->height;
   ptr->V=ptr->U+1;
   return MFX_ERR_NONE;
}

mfxStatus fa_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) {
   UNUSED_PARAM(pthis);
   UNUSED_PARAM(mid);
   if (ptr) ptr->Y=ptr->U=ptr->V=ptr->A=0;
   return MFX_ERR_NONE;
}

mfxStatus fa_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle) {
   UNUSED_PARAM(pthis);
   UNUSED_PARAM(mid);
   UNUSED_PARAM(handle);
   return MFX_ERR_UNSUPPORTED;
}

mfxStatus fa_free(mfxHDL pthis, mfxFrameAllocResponse *response) {
   UNUSED_PARAM(pthis);
   for (int i=0;i<response->NumFrameActual;i++) {
      mid_struct *mmid=(mid_struct *)response->mids[i];
      free(mmid->base); free(mmid);
   }
   return MFX_ERR_NONE;
}
/*end1*/

static void prg_mem2 () {
/*beg2*/
// lets decode frame and try to access output in an optimal way.
sts = MFXVideoDECODE_DecodeFrameAsync(session, NULL, NULL, &outsurface, &syncp);
if (MFX_ERR_NONE == sts)
{
    mfxStatus s = outsurface->FrameInterface->GetDeviceHandle(outsurface,
                                                  &device_handle, &device_type);
    // if application or component is familar with mfxHandleType and it's
    // possible to share memory created by device_handle.
    if (MFX_ERR_NONE == s && isDeviceTypeCompatible(device_type)
                          && isPossibleForMemorySharing(device_handle)) {
        // get native handle and type
        outsurface->FrameInterface->GetNativeHandle(outsurface,
                                                      &resource, &resource_type);
        if (isResourceTypeCompatible(resource_type)) {
            //use memory directly
            ProcessNativeMemory(resource);
            outsurface->FrameInterface->Release(outsurface);
        }
    } else {
      // Application or component is not aware about such DeviceHandle or
      // Resource type need to map to system memory.
      outsurface->FrameInterface->Map(outsurface, MFX_MAP_READ);
      ProcessSystemMemory(outsurface);
      outsurface->FrameInterface->Unmap(outsurface);
      outsurface->FrameInterface->Release(outsurface);
    }
}
/*end2*/
}
