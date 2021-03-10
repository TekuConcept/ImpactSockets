/**
 * Created by TekuConcept on March 6, 2021
 */

#ifndef IMPACT_ASYNC_UV_NODE_INTERFACE_H
#define IMPACT_ASYNC_UV_NODE_INTERFACE_H

#include <uv.h>

namespace impact {

    enum class uv_node_signal_t { STOP };

    class uv_child_interface {
    public:
        virtual ~uv_child_interface() = default;
        virtual void send_signal(uv_node_signal_t) = 0;
    };

    class uv_parent_interface {
    public:
        virtual ~uv_parent_interface() = default;
        virtual void add_child(uv_child_interface*) = 0;
        virtual void remove_child(uv_child_interface*) = 0;
        virtual uv_loop_t* get_loop_handle() const = 0;
    };

} /* namespace impact */

#endif /* IMPACT_ASYNC_UV_NODE_INTERFACE_H */
