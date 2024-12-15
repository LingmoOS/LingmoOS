#ifndef WL_CLIENT_ZTREELANDFOREIGNTOPLEVELMANAGEMENTV1_H
#define WL_CLIENT_ZTREELANDFOREIGNTOPLEVELMANAGEMENTV1_H

#include "Type.h"

namespace wl::client {

class ZtreelandForeignToplevelManagerV1 : public Type<ztreeland_foreign_toplevel_manager_v1>
{
public:
    explicit ZtreelandForeignToplevelManagerV1(ztreeland_foreign_toplevel_manager_v1 *val);
    ~ZtreelandForeignToplevelManagerV1();

protected:
    virtual void ztreeland_foreign_toplevel_manager_v1_toplevel(
        struct ztreeland_foreign_toplevel_handle_v1 *toplevel) = 0;
    virtual void ztreeland_foreign_toplevel_manager_v1_finished() = 0;

private:
    static const ztreeland_foreign_toplevel_manager_v1_listener listener_;
};

class ZtreelandForeignToplevelHandleV1 : public Type<ztreeland_foreign_toplevel_handle_v1>
{
public:
    explicit ZtreelandForeignToplevelHandleV1(ztreeland_foreign_toplevel_handle_v1 *val);
    ~ZtreelandForeignToplevelHandleV1();

protected:
    virtual void ztreeland_foreign_toplevel_handle_v1_pid(uint32_t pid) = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_title(const char *title) = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_app_id(const char *app_id) = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_identifier(uint identifier) = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_output_enter(struct wl_output *output) = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_output_leave(struct wl_output *output) = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_state(struct wl_array *state) = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_done() = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_closed() = 0;
    virtual void ztreeland_foreign_toplevel_handle_v1_parent(
        struct ztreeland_foreign_toplevel_handle_v1 *parent) = 0;

private:
    static const ztreeland_foreign_toplevel_handle_v1_listener listener_;
};

} // namespace wl::client

#endif // !WL_CLIENT_ZTREELANDFOREIGNTOPLEVELMANAGEMENTV1_H
