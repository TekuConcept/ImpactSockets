/**
 * Created by TekuConcept on March 8, 2018
 */

#ifndef _WSMANIP_H_
#define _WSMANIP_H_

#include <iostream>

namespace Impact {
    namespace ws {
        std::ostream& text(std::ostream& stream);
        std::ostream& binary(std::ostream& stream);
        std::ostream& ping(std::ostream& stream);
        std::ostream& close(std::ostream& stream);
        std::ostream& send(std::ostream& stream);
        std::ostream& push(std::ostream& stream);
    }
}

#endif