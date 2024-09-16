#include "XdgToplevel.h"

XdgToplevel::XdgToplevel(xdg_toplevel *val)
    : wl::client::XdgToplevel(val)
{
}

XdgToplevel::~XdgToplevel() = default;

void XdgToplevel::xdg_toplevel_configure(int32_t width, int32_t height, struct wl_array *states) { }

void XdgToplevel::xdg_toplevel_close() { }

void XdgToplevel::xdg_toplevel_configure_bounds(int32_t width, int32_t height) { }

void XdgToplevel::xdg_toplevel_wm_capabilities(struct wl_array *capabilities) { }
