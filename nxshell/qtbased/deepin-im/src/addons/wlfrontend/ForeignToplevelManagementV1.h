// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FPREIGNTOPLEVELMANAGEMENTV1_H
#define FPREIGNTOPLEVELMANAGEMENTV1_H

#include "wl/client/ZwlrForeignToplevelManagementV1.h"

#include <functional>
#include <list>
#include <memory>

namespace org::deepin::dim {

class ForeignToplevelHandleV1;

class ForeignToplevelManagerV1 : public wl::client::ZwlrForeignToplevelManagerV1
{
    friend class ForeignToplevelHandleV1;

public:
    explicit ForeignToplevelManagerV1(zwlr_foreign_toplevel_manager_v1 *val);
    ~ForeignToplevelManagerV1();

    void setRefreshCallback(const std::function<void()> &callback) { refreshCallback_ = callback; }

    std::list<std::shared_ptr<ForeignToplevelHandleV1>> &list() { return list_; };

protected:
    void zwlr_foreign_toplevel_manager_v1_toplevel(
        struct zwlr_foreign_toplevel_handle_v1 *toplevel) override;
    void zwlr_foreign_toplevel_manager_v1_finished() override;

private:
    std::list<std::shared_ptr<ForeignToplevelHandleV1>> list_;
    std::function<void()> refreshCallback_;

    void refresh();
};

class ForeignToplevelHandleV1 : public wl::client::ZwlrForeignToplevelHandleV1
{
public:
    explicit ForeignToplevelHandleV1(zwlr_foreign_toplevel_handle_v1 *val,
                                     ForeignToplevelManagerV1 *parent);
    ~ForeignToplevelHandleV1();

    const std::string &key() { return key_; }

    const std::string &appId() { return appId_; }

    bool activated() { return activated_; }

protected:
    void zwlr_foreign_toplevel_handle_v1_title(const char *title) override;
    void zwlr_foreign_toplevel_handle_v1_app_id(const char *app_id) override;
    void zwlr_foreign_toplevel_handle_v1_output_enter(struct wl_output *output) override;
    void zwlr_foreign_toplevel_handle_v1_output_leave(struct wl_output *output) override;
    void zwlr_foreign_toplevel_handle_v1_state(struct wl_array *list) override;
    void zwlr_foreign_toplevel_handle_v1_done() override;
    void zwlr_foreign_toplevel_handle_v1_closed() override;
    void
    zwlr_foreign_toplevel_handle_v1_parent(struct zwlr_foreign_toplevel_handle_v1 *parent) override;

private:
    ForeignToplevelManagerV1 *parent_;
    std::string key_;
    std::string appId_;
    bool pendingActive_ = false;
    bool activated_ = false;
};

} // namespace org::deepin::dim

#endif // !FPREIGNTOPLEVELMANAGEMENTV1_H
