#ifndef DIM_XDGTOPLEVEL_H
#define DIM_XDGTOPLEVEL_H

#include "wl/client/XdgToplevel.h"

class XdgToplevel : public wl::client::XdgToplevel
{
public:
    explicit XdgToplevel(xdg_toplevel *val);
    ~XdgToplevel() override;

protected:
    void xdg_toplevel_configure(int32_t width, int32_t height, struct wl_array *states) override;
    void xdg_toplevel_close() override;
    void xdg_toplevel_configure_bounds(int32_t width, int32_t height) override;
    void xdg_toplevel_wm_capabilities(struct wl_array *capabilities) override;
};

#endif // !DIM_XDGTOPLEVEL_H
