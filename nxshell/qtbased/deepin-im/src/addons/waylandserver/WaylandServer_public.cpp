#include "WaylandServer_public.h"

#include "addons/waylandserver/WaylandServer.h"

namespace org::deepin::dim {

namespace waylandserver {

std::shared_ptr<wl_display> getRemote(Addon *addon)
{
    auto *wls = qobject_cast<WaylandServer *>(addon);
    return wls->getRemote();
}

std::shared_ptr<wl_display> getLocal(Addon *addon)
{
    auto *wls = qobject_cast<WaylandServer *>(addon);
    return wls->getLocal();
}

std::shared_ptr<wlr_backend> getBackend(Addon *addon)
{
    auto *wls = qobject_cast<WaylandServer *>(addon);
    return wls->getBackend();
}

wl_surface *getSurface(Addon *addon)
{
    auto *wls = qobject_cast<WaylandServer *>(addon);
    return wls->getSurface();
}

std::shared_ptr<WL_ADDONS_BASE_NAMESPACE::Server> getServer(Addon *addon)
{
    auto *wls = qobject_cast<WaylandServer *>(addon);
    return wls->getServer();
}

} // namespace waylandserver

} // namespace org::deepin::dim
