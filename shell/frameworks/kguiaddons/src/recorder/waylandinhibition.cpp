/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#include "waylandinhibition_p.h"

#include <QDebug>
#include <QGuiApplication>
#include <QSharedPointer>
#include <QWaylandClientExtensionTemplate>
#include <QtWaylandClientVersion>
#include <qpa/qplatformnativeinterface.h>

#include "qwayland-keyboard-shortcuts-inhibit-unstable-v1.h"

class ShortcutsInhibitor : public QtWayland::zwp_keyboard_shortcuts_inhibitor_v1
{
public:
    ShortcutsInhibitor(::zwp_keyboard_shortcuts_inhibitor_v1 *id)
        : QtWayland::zwp_keyboard_shortcuts_inhibitor_v1(id)
    {
    }

    ~ShortcutsInhibitor() override
    {
        destroy();
    }

    void zwp_keyboard_shortcuts_inhibitor_v1_active() override
    {
        m_active = true;
    }

    void zwp_keyboard_shortcuts_inhibitor_v1_inactive() override
    {
        m_active = false;
    }

    bool isActive() const
    {
        return m_active;
    }

private:
    bool m_active = false;
};

class ShortcutsInhibitManager : public QWaylandClientExtensionTemplate<ShortcutsInhibitManager>, public QtWayland::zwp_keyboard_shortcuts_inhibit_manager_v1
{
public:
    ShortcutsInhibitManager()
        : QWaylandClientExtensionTemplate<ShortcutsInhibitManager>(1)
    {
        initialize();
    }
    ~ShortcutsInhibitManager() override
    {
        if (isInitialized()) {
            destroy();
        }
    }

    void startInhibition(QWindow *window)
    {
        if (m_inhibitions.contains(window)) {
            return;
        }
        QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
        if (!nativeInterface) {
            return;
        }
        auto seat = static_cast<wl_seat *>(nativeInterface->nativeResourceForIntegration("wl_seat"));
        auto surface = static_cast<wl_surface *>(nativeInterface->nativeResourceForWindow("surface", window));
        if (!seat || !surface) {
            return;
        }
        m_inhibitions[window].reset(new ShortcutsInhibitor(inhibit_shortcuts(surface, seat)));
    }

    bool isInhibited(QWindow *window) const
    {
        return m_inhibitions.contains(window);
    }

    void stopInhibition(QWindow *window)
    {
        m_inhibitions.remove(window);
    }

    QHash<QWindow *, QSharedPointer<ShortcutsInhibitor>> m_inhibitions;
};

static std::shared_ptr<ShortcutsInhibitManager> theManager()
{
    static std::weak_ptr<ShortcutsInhibitManager> managerInstance;
    std::shared_ptr<ShortcutsInhibitManager> ret = managerInstance.lock();
    if (!ret) {
        ret = std::make_shared<ShortcutsInhibitManager>();
        managerInstance = ret;
    }
    return ret;
}

WaylandInhibition::WaylandInhibition(QWindow *window)
    : ShortcutInhibition()
    , m_window(window)
    , m_manager(theManager())
{
}

WaylandInhibition::~WaylandInhibition() = default;

bool WaylandInhibition::shortcutsAreInhibited() const
{
    return m_manager->isInhibited(m_window);
}

void WaylandInhibition::enableInhibition()
{
    m_manager->startInhibition(m_window);
}

void WaylandInhibition::disableInhibition()
{
    m_manager->stopInhibition(m_window);
}
