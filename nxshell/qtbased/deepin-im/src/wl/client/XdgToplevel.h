#ifndef WL_CLIENT_XDGTOPLEVEL_H
#define WL_CLIENT_XDGTOPLEVEL_H

#include "Type.h"

namespace wl::client {

class XdgToplevel : public Type<xdg_toplevel>
{
public:
    XdgToplevel(xdg_toplevel *val);
    virtual ~XdgToplevel();

protected:
    /**
     * suggest a surface change
     *
     * This configure event asks the client to resize its toplevel
     * surface or to change its state. The configured state should not
     * be applied immediately. See xdg_surface.configure for details.
     *
     * The width and height arguments specify a hint to the window
     * about how its surface should be resized in window geometry
     * coordinates. See set_window_geometry.
     *
     * If the width or height arguments are zero, it means the client
     * should decide its own window dimension. This may happen when the
     * compositor needs to configure the state of the surface but
     * doesn't have any information about any previous or expected
     * dimension.
     *
     * The states listed in the event specify how the width/height
     * arguments should be interpreted, and possibly how it should be
     * drawn.
     *
     * Clients must send an ack_configure in response to this event.
     * See xdg_surface.configure and xdg_surface.ack_configure for
     * details.
     */
    virtual void xdg_toplevel_configure(int32_t width, int32_t height, struct wl_array *states) = 0;
    /**
     * surface wants to be closed
     *
     * The close event is sent by the compositor when the user wants
     * the surface to be closed. This should be equivalent to the user
     * clicking the close button in client-side decorations, if your
     * application has any.
     *
     * This is only a request that the user intends to close the
     * window. The client may choose to ignore this request, or show a
     * dialog to ask the user to save their data, etc.
     */
    virtual void xdg_toplevel_close() = 0;
    /**
     * recommended window geometry bounds
     *
     * The configure_bounds event may be sent prior to a
     * xdg_toplevel.configure event to communicate the bounds a window
     * geometry size is recommended to constrain to.
     *
     * The passed width and height are in surface coordinate space. If
     * width and height are 0, it means bounds is unknown and
     * equivalent to as if no configure_bounds event was ever sent for
     * this surface.
     *
     * The bounds can for example correspond to the size of a monitor
     * excluding any panels or other shell components, so that a
     * surface isn't created in a way that it cannot fit.
     *
     * The bounds may change at any point, and in such a case, a new
     * xdg_toplevel.configure_bounds will be sent, followed by
     * xdg_toplevel.configure and xdg_surface.configure.
     * @since 4
     */
    virtual void xdg_toplevel_configure_bounds(int32_t width, int32_t height) = 0;
    /**
     * compositor capabilities
     *
     * This event advertises the capabilities supported by the
     * compositor. If a capability isn't supported, clients should hide
     * or disable the UI elements that expose this functionality. For
     * instance, if the compositor doesn't advertise support for
     * minimized toplevels, a button triggering the set_minimized
     * request should not be displayed.
     *
     * The compositor will ignore requests it doesn't support. For
     * instance, a compositor which doesn't advertise support for
     * minimized will ignore set_minimized requests.
     *
     * Compositors must send this event once before the first
     * xdg_surface.configure event. When the capabilities change,
     * compositors must send this event again and then send an
     * xdg_surface.configure event.
     *
     * The configured state should not be applied immediately. See
     * xdg_surface.configure for details.
     *
     * The capabilities are sent as an array of 32-bit unsigned
     * integers in native endianness.
     * @param capabilities array of 32-bit capabilities
     * @since 5
     */
    virtual void xdg_toplevel_wm_capabilities(struct wl_array *capabilities) = 0;

private:
    static const xdg_toplevel_listener listener_;
};

} // namespace wl::client

#endif // !WL_CLIENT_XDGTOPLEVEL_H
