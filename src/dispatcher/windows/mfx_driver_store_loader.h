/*############################################################################
  # Copyright (C) 2019-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_DISPATCHER_WINDOWS_MFX_DRIVER_STORE_LOADER_H_
#define SRC_DISPATCHER_WINDOWS_MFX_DRIVER_STORE_LOADER_H_

#if !defined(MEDIASDK_ARM_LOADER)
    #include <cfgmgr32.h>
    #include <devguid.h>
    #include <windows.h>

    #include "mfx_dispatcher_defs.h"

namespace MFX {

typedef CONFIGRET(WINAPI *Func_CM_Get_Device_ID_List_SizeW)(PULONG pulLen,
                                                            PCWSTR pszFilter,
                                                            ULONG ulFlags);
typedef CONFIGRET(WINAPI *Func_CM_Get_Device_ID_ListW)(PCWSTR pszFilter,
                                                       PZZWSTR Buffer,
                                                       ULONG BufferLen,
                                                       ULONG ulFlags);
typedef CONFIGRET(WINAPI *Func_CM_Locate_DevNodeW)(PDEVINST pdnDevInst,
                                                   DEVINSTID_W pDeviceID,
                                                   ULONG ulFlags);
typedef CONFIGRET(WINAPI *Func_CM_Open_DevNode_Key)(DEVINST dnDevNode,
                                                    REGSAM samDesired,
                                                    ULONG ulHardwareProfile,
                                                    REGDISPOSITION Disposition,
                                                    PHKEY phkDevice,
                                                    ULONG ulFlags);

class DriverStoreLoader {
public:
    DriverStoreLoader(void);
    ~DriverStoreLoader(void);

    bool GetDriverStorePath(wchar_t *path, DWORD dwPathSize);

protected:
    bool IsIntelDeviceInstanceID(const wchar_t *DeviceID);
    bool LoadCfgMgr();
    bool LoadCmFuncs();

    mfxModuleHandle m_moduleCfgMgr;
    Func_CM_Get_Device_ID_List_SizeW m_pCM_Get_Device_ID_List_Size;
    Func_CM_Get_Device_ID_ListW m_pCM_Get_Device_ID_List;
    Func_CM_Locate_DevNodeW m_pCM_Locate_DevNode;
    Func_CM_Open_DevNode_Key m_pCM_Open_DevNode_Key;

private:
    // unimplemented by intent to make this class non-copyable
    DriverStoreLoader(const DriverStoreLoader &);
    void operator=(const DriverStoreLoader &);
};

} // namespace MFX
#endif // !defined(MEDIASDK_ARM_LOADER)

#endif // SRC_DISPATCHER_WINDOWS_MFX_DRIVER_STORE_LOADER_H_
