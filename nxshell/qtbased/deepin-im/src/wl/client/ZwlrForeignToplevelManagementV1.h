#ifndef WL_CLIENT_ZWLRFOREIGNTOPLEVELMANAGEMENTV1_H
#define WL_CLIENT_ZWLRFOREIGNTOPLEVELMANAGEMENTV1_H

#include "Type.h"

namespace wl::client {

class ZwlrForeignToplevelManagerV1 : public Type<zwlr_foreign_toplevel_manager_v1>
{
public:
    explicit ZwlrForeignToplevelManagerV1(zwlr_foreign_toplevel_manager_v1 *val);
    ~ZwlrForeignToplevelManagerV1();

protected:
    virtual void zwlr_foreign_toplevel_manager_v1_toplevel(struct zwlr_foreign_toplevel_handle_v1 *toplevel) = 0;
    virtual void zwlr_foreign_toplevel_manager_v1_finished() = 0;

private:
    static const zwlr_foreign_toplevel_manager_v1_listener listener_;
};

class ZwlrForeignToplevelHandleV1 : public Type<zwlr_foreign_toplevel_handle_v1>
{
public:
    explicit ZwlrForeignToplevelHandleV1(zwlr_foreign_toplevel_handle_v1 *val);
    ~ZwlrForeignToplevelHandleV1();

protected:
    virtual void zwlr_foreign_toplevel_handle_v1_title(const char *title) = 0;
    virtual void zwlr_foreign_toplevel_handle_v1_app_id(const char *app_id) = 0;
    virtual void zwlr_foreign_toplevel_handle_v1_output_enter(struct wl_output *output) = 0;
    virtual void zwlr_foreign_toplevel_handle_v1_output_leave(struct wl_output *output) = 0;
    virtual void zwlr_foreign_toplevel_handle_v1_state(struct wl_array *state) = 0;
    virtual void zwlr_foreign_toplevel_handle_v1_done() = 0;
    virtual void zwlr_foreign_toplevel_handle_v1_closed() = 0;
    virtual void zwlr_foreign_toplevel_handle_v1_parent(struct zwlr_foreign_toplevel_handle_v1 *parent) = 0;

private:
    static const zwlr_foreign_toplevel_handle_v1_listener listener_;
};

} // namespace wl::client

#endif // !WL_CLIENT_ZWLRFOREIGNTOPLEVELMANAGEMENTV1_H
