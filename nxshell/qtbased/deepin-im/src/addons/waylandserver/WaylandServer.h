#ifndef WAYLANDSERVER_H
#define WAYLANDSERVER_H

#include "dimcore/Addon.h"
#include "wladdonsbase/Server.h"

#include <QObject>

struct wl_display;
struct wlr_backend;

namespace wl::client {
class ConnectionBase;
}

namespace org::deepin::dim {

class WaylandServer : public Addon
{
    Q_OBJECT

public:
    explicit WaylandServer(Dim *dim);
    ~WaylandServer();

    std::shared_ptr<wl_display> getRemote();
    std::shared_ptr<wl_display> getLocal();
    std::shared_ptr<wlr_backend> getBackend();
    wl_surface *getSurface();
    std::shared_ptr<WL_ADDONS_BASE_NAMESPACE::Server> getServer();

private:
    std::shared_ptr<wl_display> remote_;
    std::shared_ptr<wl_display> local_;
    std::shared_ptr<wlr_backend> backend_;
    wl_surface *surface_ = nullptr;
    std::shared_ptr<WL_ADDONS_BASE_NAMESPACE::Server> server_;
};

} // namespace org::deepin::dim

#endif // !WAYLANDSERVER_H
