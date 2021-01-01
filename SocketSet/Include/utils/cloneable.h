/**
 * Created by TekuConcept on January 5, 2018
 */

#ifndef IMPACT_UTILS_ICLONEABLE_H
#define IMPACT_UTILS_ICLONEABLE_H

namespace impact {
    template <class T>
    class cloneable {
    public:
        virtual T&& clone() = 0;
    };
}

#endif
