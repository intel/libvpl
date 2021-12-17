/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "vpl/preview/detail/sdk_callable.hpp"
#include "vpl/preview/exception.hpp"
#include "vpl/preview/impl_caps.hpp"
#include "vpl/mfxdispatcher.h"
#include "vpl/preview/options.hpp"

namespace oneapi {
namespace vpl {

/// @brief Selects oneVPL implementation according to the specified properties.
/// @details This object iterates over the available implementations and selects an appropriate one
/// based on the @p list of properties. API user can create an instance of that class. If user
/// wants to change the implementation selection behaviour they need to subslass and
/// implement operator ().
class implementation_selector {
protected:
    /// @brief Protected ctor.
    /// @param list List of properties
    implementation_selector()
            : format_(MFX_IMPLCAPS_IMPLDESCSTRUCTURE) {}

    virtual std::vector<std::pair<std::string, detail::variant>> get_properties() const = 0;

public:
    /// @brief dtor
    virtual ~implementation_selector() {}

    /// @brief Creates session which has the requested properties. Session class object calls
    /// this method at the ctor and takes care on deletion of loader and session handles.
    /// @return Pair of loader handle and associated session handle.
    auto session() const {
        mfxStatus sts;
        implementation_capabilities_factory factory;
        auto loader = MFXLoad();

        // convert options to mfxConfig
        auto opts = get_properties();

        std::for_each(opts.begin(), opts.end(), [&](auto opt) {
            auto cfg = MFXCreateConfig(loader);
            [[maybe_unused]] detail::c_api_invoker e(detail::default_checker,
                                    MFXSetConfigFilterProperty,
                                    cfg,
                                    (const uint8_t *)opt.first.c_str(),
                                    opt.second.get_variant());
        });

        uint32_t idx = 0;
        while (true) {
            void *h;
            sts = MFXEnumImplementations(loader, idx, format_, &h);

            std::shared_ptr<void> handle(h, [&] (void *p) {
                MFXDispReleaseImplDescription(loader, p);
            });

            // break if no idx
            if (sts == MFX_ERR_NOT_FOUND)
                break;
            if (sts < 0)
                throw base_exception(sts);

            std::shared_ptr<base_implementation_capabilities> caps = factory.create(format_, h);

            if (this->operator()(caps)) {
                mfxSession s;
                detail::c_api_invoker e(detail::default_checker, MFXCreateSession, loader, idx, &s);
                return std::pair(loader, s);
            }
            idx++;
        }
        MFXUnload(loader);
        throw base_exception(MFX_ERR_NOT_INITIALIZED);
    }

protected:
    /// @brief This operator is applyed to any found oneVPL implementation. If operator returns true, a session based
    /// on found implementation is created. Otherwise, search is continued.
    /// @param caps Pointer to the session capabilities information in the requested format.
    /// @return True, if implementation is good to go, false if search must continue.
    virtual bool operator()(std::shared_ptr<base_implementation_capabilities> caps) const = 0;

    /// @brief Implementation capabilities report format
    /// @todo Replace either with enum or typename
    mfxImplCapsDeliveryFormat format_;
};

/// @brief Default implementation selector. It accepts first implementation matching provided properties.
template<typename property_collection=property_list> class default_selector : public implementation_selector {
public:
    /// @brief Protected ctor.
    /// @param list List of properties
    explicit default_selector(property_collection props = {})
            : implementation_selector(),
            props_(props) {}

    /// @brief Acccept first found implementation.
    /// @return True if implementation found.
    bool operator()(std::shared_ptr<base_implementation_capabilities>) const override {
        return true;
    }

    /// @brief Returns list of C-based properties in a form of pair: property path
    /// and property value.
    /// @return List of C-based properties
    std::vector<std::pair<std::string, detail::variant>> get_properties() const {
        return props_.get_properties();
    }

protected:
    /// @brief List of properties
    property_collection props_;
};

/// @brief Default SW based implementation selector. It accepts first implementation with SW based acceleration.
class cpu_selector : public default_selector<property_list>{
public:
    /// @brief Default ctor.
    cpu_selector()
            : default_selector({ dprops::impl(oneapi::vpl::implementation_type::sw) }) {}
};

/// @brief Default HW based implementation selector. It accepts first implementation with HW based acceleration.
class gpu_selector : public default_selector<property_list> {
public:
    /// @brief Default ctor.
    gpu_selector()
            : default_selector({ dprops::impl(oneapi::vpl::implementation_type::hw) }) {}
};

} // namespace vpl
} // namespace oneapi
