#include "ZtreelandForeignToplevelManagementV1.h"

using namespace wl::client;

const ztreeland_foreign_toplevel_manager_v1_listener ZtreelandForeignToplevelManagerV1::listener_{
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelManagerV1::ztreeland_foreign_toplevel_manager_v1_toplevel>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelManagerV1::ztreeland_foreign_toplevel_manager_v1_finished>::func,
};

ZtreelandForeignToplevelManagerV1::ZtreelandForeignToplevelManagerV1(
    ztreeland_foreign_toplevel_manager_v1 *val)
    : Type(val)
{
    ztreeland_foreign_toplevel_manager_v1_add_listener(get(), &listener_, this);
}

ZtreelandForeignToplevelManagerV1::~ZtreelandForeignToplevelManagerV1() = default;

const ztreeland_foreign_toplevel_handle_v1_listener ZtreelandForeignToplevelHandleV1::listener_{
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_pid>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_title>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_app_id>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_identifier>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_output_enter>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_output_leave>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_state>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_done>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_closed>::func,
    ListenerCallbackWrapper<
        &ZtreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_parent>::func,
};

ZtreelandForeignToplevelHandleV1::ZtreelandForeignToplevelHandleV1(
    ztreeland_foreign_toplevel_handle_v1 *val)
    : Type(val)
{
    ztreeland_foreign_toplevel_handle_v1_add_listener(get(), &listener_, this);
}

ZtreelandForeignToplevelHandleV1::~ZtreelandForeignToplevelHandleV1() = default;
