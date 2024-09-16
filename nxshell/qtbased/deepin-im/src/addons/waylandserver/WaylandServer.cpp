#include "WaylandServer.h"

#include "wl/client/Compositor.h"
#include "wl/client/ConnectionRaw.h"

extern "C" {
#define static
#include <wlr/backend/wayland.h>
#undef static
}

#include <wayland-server-core.h>

#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QSocketNotifier>
#include <QThread>

using namespace org::deepin::dim;

static const char *SOCKET_NAME = "dim";

DIM_ADDON_FACTORY(WaylandServer);

WaylandServer::WaylandServer(Dim *dim)
    : Addon(dim, "waylandserver")
    , remote_(wl_display_connect(nullptr), &wl_display_disconnect)
    , local_(wl_display_create(), &wl_display_destroy)
{
    backend_ = std::shared_ptr<wlr_backend>(wlr_wl_backend_create(local_.get(), remote_.get()),
                                            &wlr_backend_destroy);
    if (!backend_) {
        throw std::runtime_error("failed to create backend");
    }

    wlr_backend_start(backend_.get());

    wl_display_add_socket(local_.get(), SOCKET_NAME);

    auto *loop = wl_display_get_event_loop(local_.get());
    int fd = wl_event_loop_get_fd(loop);

    auto processWaylandEvents = [this, loop] {
        int ret = wl_event_loop_dispatch(loop, 0);
        if (ret) {
            qWarning() << "wl_event_loop_dispatch error:" << ret;
        }
        wl_display_flush_clients(local_.get());
    };

    auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read);
    QObject::connect(notifier, &QSocketNotifier::activated, processWaylandEvents);

    QAbstractEventDispatcher *dispatcher = QThread::currentThread()->eventDispatcher();
    QObject::connect(dispatcher, &QAbstractEventDispatcher::aboutToBlock, processWaylandEvents);

    auto remote1 = std::make_shared<wl::client::ConnectionRaw>(remote_.get());
    auto compositor = remote1->getGlobal<wl::client::Compositor>();
    surface_ = compositor->create_surface();

    server_ = std::make_shared<WL_ADDONS_BASE_NAMESPACE::Server>(local_, backend_);
    server_->createOutputFromSurface(surface_);
}

WaylandServer::~WaylandServer() = default;

std::shared_ptr<wl_display> WaylandServer::getRemote()
{
    return remote_;
}

std::shared_ptr<wl_display> WaylandServer::getLocal()
{
    return local_;
}

std::shared_ptr<wlr_backend> WaylandServer::getBackend()
{
    return backend_;
}

wl_surface *WaylandServer::getSurface()
{
    return surface_;
}

std::shared_ptr<WL_ADDONS_BASE_NAMESPACE::Server> WaylandServer::getServer()
{
    return server_;
}
