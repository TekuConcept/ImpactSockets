/**
 * Created by TekuConcept on January 3, 2021
 */

#include "interfaces/event_loop_interface.h"
#include "async/uv_event_loop.h"

namespace impact {

    static std::shared_ptr<event_loop_interface>
    s_default_event_loop = nullptr;

    std::shared_ptr<event_loop_interface>
    default_event_loop()
    {
        // TODO: create event-loop that does not depend
        //       on external libraries
        if (s_default_event_loop == nullptr)
            s_default_event_loop = std::make_shared<uv_event_loop>();
        return s_default_event_loop;
    }

    void
    default_event_loop(std::shared_ptr<event_loop_interface> __event_loop)
    { s_default_event_loop = __event_loop; }

} /* namespace impact */
