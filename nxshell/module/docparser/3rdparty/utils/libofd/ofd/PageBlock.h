#ifndef __OFD_PAGEBLOCK_H__
#define __OFD_PAGEBLOCK_H__

#include <memory>
#include <string>
#include <vector>
#include "ofd/Common.h"

namespace ofd{

    // ======== class PageBlock ========
    // OFD (section 7.7) P21，Page.xsd。
    class PageBlock : public std::enable_shared_from_this<PageBlock> {
        public:
            PageBlock();
            virtual ~PageBlock();


    }; // class PageBlock
    typedef std::shared_ptr<PageBlock> PageBlockPtr;

}; // namespace ofd

#endif // __OFD_PAGEBLOCK_H__
