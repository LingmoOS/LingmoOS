#include "XdgToplevel.h"

using namespace wl::client;

const xdg_toplevel_listener XdgToplevel::listener_{
    ListenerCallbackWrapper<&XdgToplevel::xdg_toplevel_configure>::func,
    ListenerCallbackWrapper<&XdgToplevel::xdg_toplevel_close>::func,
    ListenerCallbackWrapper<&XdgToplevel::xdg_toplevel_configure_bounds>::func,
    ListenerCallbackWrapper<&XdgToplevel::xdg_toplevel_wm_capabilities>::func,
};

XdgToplevel::XdgToplevel(xdg_toplevel *val)
    : Type(val)
{
    xdg_toplevel_add_listener(get(), &listener_, this);
}

XdgToplevel::~XdgToplevel() = default;
