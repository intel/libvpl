# System analyzer utility 

This Linux-only utility provides a starting point to check runtime environment readiness and troubleshoot issues. 
Intent is that it will expand in scope to provide more features and diagnostics.

## Approach 

OS-specific environment checks are attempted, with feedback about possible failure origins.
Linux: 
 * presence of /dev/dri render nodes to indicate that the OS can communicate with adapters
 * data available from oneVPL implementations found by MFXLoad

## Example output
```
------------------------------------
Looking for GPU interfaces available to OS...
FOUND: /dev/dri/renderD128
FOUND: /dev/dri/renderD129
GPU interfaces found: 2
------------------------------------


------------------------------------
Available implementation details:

Implementation #0: mfx-gen
  Library path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.7
  AccelerationMode: MFX_ACCEL_MODE_VIA_VAAPI
  ApiVersion: 2.7
  Impl: MFX_IMPL_TYPE_HARDWARE
  ImplName: mfx-gen
  MediaAdapterType: MFX_MEDIA_INTEGRATED
  VendorID: 0x8086
  DeviceID: 0x9A49
  GPU name: Intel® Iris® Xe Graphics GT2 (arch=Xe codename=Tiger Lake)
  PCI BDF: 0000:00:02.00
  DRMRenderNodeNum: 128
  DeviceName: mfx-gen

Implementation #1: mfx-gen
  Library path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.7
  AccelerationMode: MFX_ACCEL_MODE_VIA_VAAPI
  ApiVersion: 2.7
  Impl: MFX_IMPL_TYPE_HARDWARE
  ImplName: mfx-gen
  MediaAdapterType: MFX_MEDIA_DISCRETE
  VendorID: 0x8086
  DeviceID: 0x4905
  GPU name: Intel® Iris® Xe MAX Graphics (arch=Xe MAX codename=DG1)
  PCI BDF: 0000:03:00.00
  DRMRenderNodeNum: 129
  DeviceName: mfx-gen
------------------------------------

## Plans

Windows will be covered in future updates.  Currently priority for Windows is lower as there are fewer issues with basic Windows setup as well as fewer interop scenarios enabled.
