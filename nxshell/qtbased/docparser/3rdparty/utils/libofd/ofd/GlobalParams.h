#ifndef __OFD_GLOBALPARAMS_H__
#define __OFD_GLOBALPARAMS_H__

#include "ofd/Common.h"

namespace ofd {

    class GlobalParams{
        public:
            GlobalParams();
            virtual ~GlobalParams();
            static GlobalParams globalInstance;

            // =============== Public Attributes ================
        public:

    }; // GlobalParams;

}; // namespace ofd

#endif // __OFD_GLOBALPARAMS_H__
