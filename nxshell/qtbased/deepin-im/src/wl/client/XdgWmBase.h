#ifndef WL_CLIENT_XDGWMBASE_H
#define WL_CLIENT_XDGWMBASE_H

#include "Type.h"

namespace wl::client {

class XdgWmBase : public Type<xdg_wm_base>
{
public:
    explicit XdgWmBase(xdg_wm_base *val);
    ~XdgWmBase();
};

} // namespace wl::client

#endif // !WL_CLIENT_XDGWMBASE_H
