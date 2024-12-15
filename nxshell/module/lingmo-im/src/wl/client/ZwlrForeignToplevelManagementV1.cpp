#include "ZwlrForeignToplevelManagementV1.h"

using namespace wl::client;

const zwlr_foreign_toplevel_manager_v1_listener ZwlrForeignToplevelManagerV1::listener_{
    ListenerCallbackWrapper<&ZwlrForeignToplevelManagerV1::zwlr_foreign_toplevel_manager_v1_toplevel>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelManagerV1::zwlr_foreign_toplevel_manager_v1_finished>::func,
};

ZwlrForeignToplevelManagerV1::ZwlrForeignToplevelManagerV1(zwlr_foreign_toplevel_manager_v1 *val)
    : Type(val)
{
    zwlr_foreign_toplevel_manager_v1_add_listener(get(), &listener_, this);
}

ZwlrForeignToplevelManagerV1::~ZwlrForeignToplevelManagerV1() = default;

const zwlr_foreign_toplevel_handle_v1_listener ZwlrForeignToplevelHandleV1::listener_{
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_title>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_app_id>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_output_enter>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_output_leave>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_state>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_done>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_closed>::func,
    ListenerCallbackWrapper<&ZwlrForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_parent>::func,
};

ZwlrForeignToplevelHandleV1::ZwlrForeignToplevelHandleV1(zwlr_foreign_toplevel_handle_v1 *val) : Type(val) {
    zwlr_foreign_toplevel_handle_v1_add_listener(get(), &listener_, this);
}

ZwlrForeignToplevelHandleV1::~ZwlrForeignToplevelHandleV1() = default;
