#include "TreelandForeignToplevelManagementV1.h"

#include <iomanip>

using namespace org::deepin::dim;

TreelandForeignToplevelManagerV1::TreelandForeignToplevelManagerV1(
    ztreeland_foreign_toplevel_manager_v1 *val)
    : wl::client::ZtreelandForeignToplevelManagerV1(val)
{
}

TreelandForeignToplevelManagerV1::~TreelandForeignToplevelManagerV1() = default;

void TreelandForeignToplevelManagerV1::ztreeland_foreign_toplevel_manager_v1_toplevel(
    struct ztreeland_foreign_toplevel_handle_v1 *toplevel)
{
    list_.emplace_back(std::make_shared<TreelandForeignToplevelHandleV1>(toplevel, this));
}

void TreelandForeignToplevelManagerV1::ztreeland_foreign_toplevel_manager_v1_finished() { }

void TreelandForeignToplevelManagerV1::refresh()
{
    if (refreshCallback_) {
        refreshCallback_();
    }
}

TreelandForeignToplevelHandleV1::TreelandForeignToplevelHandleV1(
    ztreeland_foreign_toplevel_handle_v1 *val, TreelandForeignToplevelManagerV1 *parent)
    : wl::client::ZtreelandForeignToplevelHandleV1(val)
    , parent_(parent)
{
    const void *addr = static_cast<const void *>(this);
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(sizeof(addr) * 2) << std::hex << addr;
    key_ = stream.str();
}

TreelandForeignToplevelHandleV1 ::~TreelandForeignToplevelHandleV1() { }

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_pid(uint32_t pid) { }

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_title(const char *title)
{
}

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_app_id(
    const char *app_id)
{
    appId_ = app_id;
    parent_->refresh();
}

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_identifier(
    uint identifier)
{
}

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_output_enter(
    struct wl_output *output)
{
}

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_output_leave(
    struct wl_output *output)
{
}

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_state(
    struct wl_array *list)
{
    pendingActive_ = false;
    size_t size = list->size / sizeof(uint32_t);
    for (size_t i = 0; i < size; ++i) {
        auto entry = static_cast<uint32_t *>(list->data)[i];
        if (entry == ZTREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
            pendingActive_ = true;
        }
    }
}

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_done()
{
    activated_ = pendingActive_;
    parent_->refresh();
}

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_closed() { }

void TreelandForeignToplevelHandleV1::ztreeland_foreign_toplevel_handle_v1_parent(
    struct ztreeland_foreign_toplevel_handle_v1 *parent)
{
}
