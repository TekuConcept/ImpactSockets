/**
 * Created by TekuConcept on January 5, 2018
 */

#ifndef _IMPACT_UTILS_ICLONEABLE_H_
#define _IMPACT_UTILS_ICLONEABLE_H_

namespace impact {
    template <class T>
    class cloneable {
    public:
        virtual T&& clone() = 0;
    };
}

#endif
