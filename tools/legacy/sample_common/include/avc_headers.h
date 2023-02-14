/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __AVC_HEADERS_H
#define __AVC_HEADERS_H

#include <vector>
#include "avc_structures.h"

namespace ProtectedLibrary {

template <typename T>
class HeaderSet {
public:
    HeaderSet() : m_header(), m_currentID(-1) {}

    ~HeaderSet() {
        Reset();
    }

    void AddHeader(T* hdr) {
        mfxU32 id = hdr->GetID();
        if (id >= m_header.size()) {
            m_header.resize(id + 1);
        }

        delete m_header[id];

        m_header[id]    = new T();
        *(m_header[id]) = *hdr;
    }

    T* GetHeader(mfxU32 id) {
        if (id >= m_header.size())
            return 0;

        return m_header[id];
    }

    void RemoveHeader(mfxU32 id) {
        if (id >= m_header.size())
            return;

        delete m_header[id];
        m_header[id] = 0;
    }

    void RemoveHeader(T* hdr) {
        if (!hdr)
            return;

        RemoveHeader(hdr->GetID());
    }

    const T* GetHeader(mfxU32 id) const {
        if (id >= m_header.size())
            return 0;

        return m_header[id];
    }

    void Reset() {
        for (mfxU32 i = 0; i < m_header.size(); i++) {
            delete m_header[i];
            m_header[i] = 0;
        }
    }

    void SetCurrentID(mfxU32 id) {
        m_currentID = id;
    }

    mfxI32 GetCurrrentID() const {
        return m_currentID;
    }

    T* GetCurrentHeader() {
        if (m_currentID == -1)
            return 0;

        return GetHeader(m_currentID);
    }

    const T* GetCurrentHeader() const {
        if (m_currentID == -1)
            return 0;

        return GetHeader(m_currentID);
    }

private:
    std::vector<T*> m_header;
    mfxI32 m_currentID;
};

/****************************************************************************************************/
// Headers stuff
/****************************************************************************************************/
class AVCHeaders {
public:
    void Reset() {
        m_SeqParams.Reset();
        m_SeqExParams.Reset();
        m_SeqParamsMvcExt.Reset();
        m_PicParams.Reset();
        m_SEIParams.Reset();
    }

    HeaderSet<AVCSeqParamSet> m_SeqParams;
    HeaderSet<AVCSeqParamSetExtension> m_SeqExParams;
    HeaderSet<AVCSeqParamSet> m_SeqParamsMvcExt;
    HeaderSet<AVCPicParamSet> m_PicParams;
    HeaderSet<AVCSEIPayLoad> m_SEIParams;
    AVCNalExtension m_nalExtension;
};

} //namespace ProtectedLibrary

#endif // __AVC_HEADERS_H
