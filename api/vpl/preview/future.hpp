/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <chrono>
#include <deque>
#include <memory>
#include <string>
#include <utility>

#include "vpl/preview/bitstream.hpp"
#include "vpl/preview/defs.hpp"
#include "vpl/preview/frame_surface.hpp"

namespace oneapi {
namespace vpl {

/// @brief This structure represents schedulling status of the asyncroniouse execution of single
/// processing (decode/encode/vpp)
/// operation.
struct operation_status {
    /// @brief Ctor. Initializes structure with default values.
    /// @param[in] component Type of the component generated from the status.
    /// @param[in] owner Pointer to the component generated from this status.
    operation_status(component component, void *owner)
            : schedule_status_(status::Unknown),
              exec_status_(status::Unknown),
              fatal_(false),
              component_(component),
              owner_(owner) {}

    /// @brief Operation's schedulling status for the execution.
    status schedule_status_;
    /// @todo not in use. Candidate for removal.
    status exec_status_;
    /// @brief Flag to indicate that fatal error occured. See schedule_status_ value for the status code.
    bool fatal_;
    /// @brief Domain of the component which generated this operation.
    component component_;
    /// @brief Pointer to the Session class.
    /// @todo need to define session_base class and use its type here instead of void.
    void *owner_;
};

inline std::ostream &operator<<(std::ostream &out, const operation_status &o) {
    out << detail::space(detail::INTENT, out, "Component = ")
        << detail::component2String(o.component_) << std::endl;
    out << detail::space(detail::INTENT, out, "SS        = ")
        << detail::status2Str(o.schedule_status_) << std::endl;
    out << detail::space(detail::INTENT, out, "ES        = ") << detail::status2Str(o.exec_status_)
        << std::endl;
    out << detail::space(detail::INTENT, out, "Fatalilty = ") << std::boolalpha << o.fatal_
        << std::noboolalpha << std::endl;
    return out;
}

/// @brief This class represent future data container and used to glue processing of the individual components
/// into the pipeline. Once component which is down in the pipeline recieved that object, it must use it to wait for
/// the data. States of the data in this object:
/// @li Not sync'd. Use the wait or wait_for methods to sync it.
/// @li Data is ready. Use Map method to get it mapped to the system memory.
/// @li More data is required. Data processing buffered in the pipe above the element. This means
///    that data won't be delivered into this future. Stop it's usage.
/// @li End of stream is reached. Future won't contain any data. Close the pipeline processing.
/// @tparam data frame_surface class or bitstream_as_dst class
/// @todo Remove "more data is required" state.
template <typename data,
          typename = typename std::enable_if<
              std::is_base_of<std::shared_ptr<frame_surface>, data>::value ||
              std::is_base_of<std::shared_ptr<bitstream_as_dst>, data>::value>::type>
class future {
public:
    /// @brief Default ctor
    /// @param[in] future_data Data object to take care about.
    explicit future(data future_data) : data_(future_data), fatal_happened_(false) {}

    /// @brief Indefinitely waits for operation completion.
    void wait() {
        if (have_to_wait() && data_) {
            data_->wait();
        }
    }

    /// @brief Provides syncronized data. Waits indefinitely for the synchronization.
    /// @return Synchronized data. Use Map method of the data container to access the data.
    data &get() {
        wait();
        return data_;
    }
    /// @brief Waits for the operation completion. Waits for the result to become available. Blocks until specified
    /// timeout_duration has elapsed or the result becomes available, whichever comes first. Returns value identifying
    /// the state of the result.
    /// @param timeout_duration Maximum duration to block for.
    /// @return Wait ststus.
    template <class Rep, class Period>
    async_op_status wait_for(const std::chrono::duration<Rep, Period> &timeout_duration) const {
        if (have_to_wait() && data_)
            return data_->wait_for(timeout_duration);
        return async_op_status::cancelled;
    }

    /// @brief add current operation scheduling status into the history of the future.
    /// @param[in] op Operation's status
    void add_operation(operation_status op) {
        history_.push_back(op);
        fatal_happened_ = op.fatal_;
    }

    /// @brief retrieve last operation scheduling status
    /// @return operation scheduling status
    status get_last_schedule_status() {
        return history_.back().schedule_status_;
    }

    /// @brief retrieve last operation exec status
    /// @return operation exec status
    status get_last_exec_status() {
        return history_.back().exec_status_;
    }

    /// @brief Check if fatal error happened.
    /// @return true if fatal error happened.
    bool had_fatal() {
        return fatal_happened_;
    }

    /// @brief Returns components with fatal status.
    /// @return Components with fatal status.
    component get_fatal_component() {
        component c = component::unknown;
        std::for_each(history_.rbegin(), history_.rend(), [&](operation_status s) {
            if (s.fatal_)
                return c = s.component_;
        });
        return c;
    }

    /// @brief Propagate processing history from previous future object in the pipeline.
    /// @param old Reference to the previouse future object in the pipeline
    /// @tparam T Type of the data container
    template <typename T>
    void propagate_history(const future<T> &old) {
        std::for_each(old.history_.rbegin(), old.history_.rend(), [&](operation_status s) {
            history_.push_front(s);
        });
    }

    /// Processing history
    std::deque<operation_status> history_;

protected:
    /// @brief Checks if we need to wait for the data or skip the processing.
    /// @return true if wait operation is required.
    bool have_to_wait() const {
        if (history_.empty())
            return false;
        return ((status::Ok == history_.back().schedule_status_) &&
                (false == history_.back().fatal_));
    }
    /// Data container
    data data_;

    /// Global fatal flag. Updated when first operation in the pipeline provided fatal status code.
    bool fatal_happened_;

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] p Reference to the future instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const future<data> &p) {
        std::string mangled_name(typeid(data).name());
        if (auto it = mangled_name.find("bitstream_as_dst"); it != std::string::npos) {
            out << "Future data type: "
                << "Bitstream" << std::endl;
        }
        else {
            out << "Future data type: "
                << "Frame" << std::endl;
        }
        out << detail::space(detail::INTENT, out, "History:") << std::endl;
        for (auto it : p.history_) {
            out << it << std::endl;
        }
        return out;
    }
};

using future_surface_t   = future<std::shared_ptr<frame_surface>>;
using future_bitstream_t = future<std::shared_ptr<bitstream_as_dst>>;

} // namespace vpl
} // namespace oneapi
