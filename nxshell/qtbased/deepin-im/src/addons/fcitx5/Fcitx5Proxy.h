// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FCITX5PROXY_H
#define FCITX5PROXY_H

#include "wladdonsbase/InputPopupSurfaceV2.h"
#include "wladdonsbase/Global.h"
#include "wladdonsbase/Server.h"

#include <dimcore/ProxyAddon.h>

#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QProcess>

namespace org {
namespace deepin {
namespace dim {

class DBusProvider;

class Fcitx5Proxy : public ProxyAddon
{
public:
    explicit Fcitx5Proxy(Dim *dim);
    ~Fcitx5Proxy() override;

    void initInputMethods() override;
    const QList<InputMethodEntry> &getInputMethods() override;
    bool keyEvent(const InputMethodEntry &entry, InputContextKeyEvent &keyEvent) override;

    void focusIn(uint32_t id) override;
    void focusOut(uint32_t id) override;
    void destroyed(uint32_t id) override;
    void done() override;
    void contentType(uint32_t hint, uint32_t purpose) override;
    void updateSurroundingText(InputContextEvent &event) override;
    void cursorRectangleChangeEvent(InputContextCursorRectChangeEvent &event) override;
    void setCurrentIM(const std::string &im) override;

private:
    void updateInputMethods();
    bool shouldBeIgnored(const std::string &uniqueName) const;
    void initDBusConn();
    void launchDaemon();
    InputContext* getFocusedIC(uint32_t id) const;

private:
    std::shared_ptr<WL_ADDONS_BASE_NAMESPACE::Server> wl_;
    uint32_t focusedId_;
    std::unique_ptr<InputPopupSurfaceV2> popup_;

    DBusProvider *dbusProvider_ = nullptr;
    bool available_ = false;
    QList<InputMethodEntry> inputMethods_;
    QProcess *fcitx5Proc_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !FCITX5PROXY_H
