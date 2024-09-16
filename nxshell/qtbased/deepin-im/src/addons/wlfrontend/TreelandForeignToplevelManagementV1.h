// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELANDFOREIGNTOPLEVELMANAGEMENTV1_H
#define TREELANDFOREIGNTOPLEVELMANAGEMENTV1_H

#include "wl/client/ZtreelandForeignToplevelManagementV1.h"

#include <functional>
#include <list>
#include <memory>

namespace org::deepin::dim {

class TreelandForeignToplevelHandleV1;

class TreelandForeignToplevelManagerV1 : public wl::client::ZtreelandForeignToplevelManagerV1
{
    friend class TreelandForeignToplevelHandleV1;

public:
    explicit TreelandForeignToplevelManagerV1(ztreeland_foreign_toplevel_manager_v1 *val);
    ~TreelandForeignToplevelManagerV1();

    void setRefreshCallback(const std::function<void()> &callback) { refreshCallback_ = callback; }

    std::list<std::shared_ptr<TreelandForeignToplevelHandleV1>> &list() { return list_; };

protected:
    void ztreeland_foreign_toplevel_manager_v1_toplevel(
        struct ztreeland_foreign_toplevel_handle_v1 *toplevel) override;
    void ztreeland_foreign_toplevel_manager_v1_finished() override;

private:
    std::list<std::shared_ptr<TreelandForeignToplevelHandleV1>> list_;
    std::function<void()> refreshCallback_;

    void refresh();
};

class TreelandForeignToplevelHandleV1 : public wl::client::ZtreelandForeignToplevelHandleV1
{
public:
    explicit TreelandForeignToplevelHandleV1(ztreeland_foreign_toplevel_handle_v1 *val,
                                             TreelandForeignToplevelManagerV1 *parent);
    ~TreelandForeignToplevelHandleV1();

    const std::string &key() { return key_; }

    const std::string &appId() { return appId_; }

    bool activated() { return activated_; }

protected:
    void ztreeland_foreign_toplevel_handle_v1_pid(uint32_t pid) override;
    void ztreeland_foreign_toplevel_handle_v1_title(const char *title) override;
    void ztreeland_foreign_toplevel_handle_v1_app_id(const char *app_id) override;
    void ztreeland_foreign_toplevel_handle_v1_identifier(uint identifier) override;
    void ztreeland_foreign_toplevel_handle_v1_output_enter(struct wl_output *output) override;
    void ztreeland_foreign_toplevel_handle_v1_output_leave(struct wl_output *output) override;
    void ztreeland_foreign_toplevel_handle_v1_state(struct wl_array *list) override;
    void ztreeland_foreign_toplevel_handle_v1_done() override;
    void ztreeland_foreign_toplevel_handle_v1_closed() override;
    void ztreeland_foreign_toplevel_handle_v1_parent(
        struct ztreeland_foreign_toplevel_handle_v1 *parent) override;

private:
    TreelandForeignToplevelManagerV1 *parent_;
    std::string key_;
    std::string appId_;
    bool pendingActive_ = false;
    bool activated_ = false;
};

} // namespace org::deepin::dim

#endif // !TREELANDFOREIGNTOPLEVELMANAGEMENTV1_H
