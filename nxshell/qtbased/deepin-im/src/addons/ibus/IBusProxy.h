// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IBUSPROXY_H
#define IBUSPROXY_H

#include "IBUSInputContextIface.h"
#include "common/common.h"
#include "wladdonsbase/Global.h"
#include "wladdonsbase/InputPopupSurfaceV2.h"
#include "wladdonsbase/Server.h"

#include <dimcore/ProxyAddon.h>
#include <gio/gio.h>

#include <QProcess>
#include <QTimer>

#include <memory>

class IBusEngineDesc;
class DimIBusInputContextPrivate;

namespace org {
namespace deepin {
namespace dim {

class DimIBusProxy : public ProxyAddon
{
public:
    explicit DimIBusProxy(Dim *dim);
    ~DimIBusProxy() override;

    const QList<InputMethodEntry> &getInputMethods() override;
    void initInputMethods() override;
    bool keyEvent(const InputMethodEntry &entry, InputContextKeyEvent &keyEvent) override;

    void focusIn(uint32_t id) override;
    void focusOut(uint32_t id) override;
    void destroyed(uint32_t id) override;
    void done() override;
    void contentType(uint32_t hint, uint32_t purpose) override;
    void updateSurroundingText(InputContextEvent &event) override;
    void cursorRectangleChangeEvent(InputContextCursorRectChangeEvent &event) override;
    void setCurrentIM(const std::string &im) override;

    void forwardKey(uint32_t keycode, uint32_t state);
    void commit(uint32_t serial, const char *text);
    void preedit(uint32_t serial, const char *text, const char *commit);
    void panelCreate();
    void panelDestroy();

private:
    QList<IBusEngineDesc> listEngines();
    void initEngines();

    inline bool isICDBusInterfaceValid(uint32_t id)
    {
        return !iBusICMap_.isEmpty() && iBusICMap_.contains(id) && iBusICMap_[id]->isValid();
    }

    void launchDaemon();

public Q_SLOTS:
    void connectToBus();
    void socketChanged(const QString &str);
    void busRegistered(const QString &str);
    void busUnregistered(const QString &str);
    void globalEngineChanged(const QString &engineName);

private:
    bool shouldBeIgnored(const std::string &uniqueName) const;
    void stopInputMethod();
    InputContext *getFocusedIC(uint32_t id) const;

private:
    std::unique_ptr<GSettings, Deleter<g_object_unref>> gsettings_;
    std::unique_ptr<DimIBusInputContextPrivate> d;
    bool useSyncMode_;
    bool daemonLaunched_;
    QFileSystemWatcher socketWatcher_;
    QTimer timer_;
    QList<InputMethodEntry> inputMethods_;
    QMap<uint32_t, std::shared_ptr<OrgFreedesktopIBusInputContextInterface>> iBusICMap_;
    QProcess *ibusDaemonProc_ = nullptr;
    std::shared_ptr<WL_ADDONS_BASE_NAMESPACE::Server> wl_;
    QTimer daemonCrashTimer_;
    uint daemonCrashes_ = 0;
    uint32_t focusedId_ = 0;
    std::unique_ptr<InputPopupSurfaceV2> popup_;
    wl_surface * surface_ = nullptr;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // IBUSPROXY_H