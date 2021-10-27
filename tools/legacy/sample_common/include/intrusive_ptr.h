/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

//intrusive ptr concept
//usage examples same as smart pointers except user has to define addref and release routine for that class

//inline void intrusive_ptr_addref(UserClassA * pResource);
//inline void intrusive_ptr_release(UserClassA * pResource);

template <class T>
class intrusive_ptr {
    T* m_pResource;

public:
    intrusive_ptr(T* pResource = NULL) : m_pResource(pResource) {
        intrusive_ptr_addref(m_pResource);
    }
    intrusive_ptr(const intrusive_ptr<T>& rhs) : m_pResource(rhs.m_pResource) {
        intrusive_ptr_addref(m_pResource);
    }
    void reset(T* pResource) {
        if (m_pResource) {
            intrusive_ptr_release(m_pResource);
        }
        m_pResource = pResource;
        intrusive_ptr_addref(m_pResource);
    }
    T* operator*() {
        return m_pResource;
    }
    T* operator->() {
        return m_pResource;
    }
    T* get() {
        return m_pResource;
    }
    ~intrusive_ptr() {
        if (m_pResource) {
            intrusive_ptr_release(m_pResource);
        }
    }
};
