#ifndef WL_CLIENT_XDGSURFACE_H
#define WL_CLIENT_XDGSURFACE_H

#include "Type.h"

namespace wl::client {

class XdgSurface : public Type<xdg_surface>
{
public:
    XdgSurface(xdg_surface *val);
    virtual ~XdgSurface();

protected:
    /**
     * suggest a surface change
     *
     * The configure event marks the end of a configure sequence. A
     * configure sequence is a set of one or more events configuring
     * the state of the xdg_surface, including the final
     * xdg_surface.configure event.
     *
     * Where applicable, xdg_surface surface roles will during a
     * configure sequence extend this event as a latched state sent as
     * events before the xdg_surface.configure event. Such events
     * should be considered to make up a set of atomically applied
     * configuration states, where the xdg_surface.configure commits
     * the accumulated state.
     *
     * Clients should arrange their surface for the new states, and
     * then send an ack_configure request with the serial sent in this
     * configure event at some point before committing the new surface.
     *
     * If the client receives multiple configure events before it can
     * respond to one, it is free to discard all but the last event it
     * received.
     * @param serial serial of the configure event
     */
    virtual void xdg_surface_configure(uint32_t serial) = 0;

private:
    static const xdg_surface_listener listener_;
};

} // namespace wl::client

#endif // !WL_CLIENT_XDGSURFACE_H
