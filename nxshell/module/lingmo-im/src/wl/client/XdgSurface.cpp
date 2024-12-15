#include "XdgSurface.h"

using namespace wl::client;

const xdg_surface_listener XdgSurface::listener_{
    ListenerCallbackWrapper<&XdgSurface::xdg_surface_configure>::func,
};

XdgSurface::XdgSurface(xdg_surface *val)
    : Type(val)
{
    xdg_surface_add_listener(get(), &listener_, this);
}

XdgSurface::~XdgSurface() = default;
