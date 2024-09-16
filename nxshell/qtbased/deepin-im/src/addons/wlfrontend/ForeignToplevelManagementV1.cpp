#include "ForeignToplevelManagementV1.h"

#include <iomanip>

using namespace org::deepin::dim;

ForeignToplevelManagerV1::ForeignToplevelManagerV1(zwlr_foreign_toplevel_manager_v1 *val)
    : wl::client::ZwlrForeignToplevelManagerV1(val)
{
}

ForeignToplevelManagerV1::~ForeignToplevelManagerV1() = default;

void ForeignToplevelManagerV1::zwlr_foreign_toplevel_manager_v1_toplevel(
    struct zwlr_foreign_toplevel_handle_v1 *toplevel)
{
    list_.emplace_back(std::make_shared<ForeignToplevelHandleV1>(toplevel, this));
}

void ForeignToplevelManagerV1::zwlr_foreign_toplevel_manager_v1_finished() { }

void ForeignToplevelManagerV1::refresh()
{
    if (refreshCallback_) {
        refreshCallback_();
    }
}

ForeignToplevelHandleV1::ForeignToplevelHandleV1(zwlr_foreign_toplevel_handle_v1 *val,
                                                 ForeignToplevelManagerV1 *parent)
    : wl::client::ZwlrForeignToplevelHandleV1(val)
    , parent_(parent)
{
    const void *addr = static_cast<const void *>(this);
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(sizeof(addr) * 2) << std::hex << addr;
    key_ = stream.str();
}

ForeignToplevelHandleV1 ::~ForeignToplevelHandleV1() { }

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_title(const char *title) { }

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_app_id(const char *app_id)
{
    appId_ = app_id;
    parent_->refresh();
}

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_output_enter(struct wl_output *output)
{
}

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_output_leave(struct wl_output *output)
{
}

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_state(struct wl_array *list)
{
    pendingActive_ = false;
    size_t size = list->size / sizeof(uint32_t);
    for (size_t i = 0; i < size; ++i) {
        auto entry = static_cast<uint32_t *>(list->data)[i];
        if (entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
            pendingActive_ = true;
        }
    }
}

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_done()
{
    activated_ = pendingActive_;
    parent_->refresh();
}

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_closed() { }

void ForeignToplevelHandleV1::zwlr_foreign_toplevel_handle_v1_parent(
    struct zwlr_foreign_toplevel_handle_v1 *parent)
{
}
