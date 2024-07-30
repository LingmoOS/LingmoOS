/*
 *  SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "tabletmodewatcher.h"
#include <QCoreApplication>

#if defined(LINGMOUI_ENABLE_DBUS)
#include "settings_interface.h"
#include <QDBusConnection>
#endif

using namespace Qt::Literals::StringLiterals;

// TODO: All the dbus stuff should be conditional, optional win32 support

namespace LingmoUI
{
namespace Platform
{

class TabletModeWatcherSingleton
{
public:
    TabletModeWatcher self;
};

Q_GLOBAL_STATIC(TabletModeWatcherSingleton, privateTabletModeWatcherSelf)

class TabletModeWatcherPrivate
{
    static constexpr auto PORTAL_GROUP = "org.kde.TabletMode"_L1;
    static constexpr auto KEY_AVAILABLE = "available"_L1;
    static constexpr auto KEY_ENABLED = "enabled"_L1;

public:
    TabletModeWatcherPrivate(TabletModeWatcher *watcher)
        : q(watcher)
    {
        // Called here to avoid collisions with application event types so we should use
        // registerEventType for generating the event types.
        TabletModeChangedEvent::type = QEvent::Type(QEvent::registerEventType());
#if !defined(LINGMOUI_ENABLE_DBUS) && (defined(Q_OS_ANDROID) || defined(Q_OS_IOS))
        isTabletModeAvailable = true;
        isTabletMode = true;
#elif defined(LINGMOUI_ENABLE_DBUS)
        // Mostly for debug purposes and for platforms which are always mobile,
        // such as Lingmo Mobile
        if (qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_MOBILE") || qEnvironmentVariableIsSet("KDE_LINGMOUI_TABLET_MODE")) {
            /* clang-format off */
            isTabletMode =
                (QString::fromLatin1(qgetenv("QT_QUICK_CONTROLS_MOBILE")) == QStringLiteral("1")
                    || QString::fromLatin1(qgetenv("QT_QUICK_CONTROLS_MOBILE")) == QStringLiteral("true"))
                    || (QString::fromLatin1(qgetenv("KDE_LINGMOUI_TABLET_MODE")) == QStringLiteral("1")
                    || QString::fromLatin1(qgetenv("KDE_LINGMOUI_TABLET_MODE")) == QStringLiteral("true"));
            /* clang-format on */
            isTabletModeAvailable = isTabletMode;
        } else if (qEnvironmentVariableIsSet("QT_NO_XDG_DESKTOP_PORTAL")) {
            isTabletMode = false;
        } else {
            qDBusRegisterMetaType<VariantMapMap>();
            auto portal = new OrgFreedesktopPortalSettingsInterface(u"org.freedesktop.portal.Desktop"_s,
                                                                    u"/org/freedesktop/portal/desktop"_s,
                                                                    QDBusConnection::sessionBus(),
                                                                    q);

            QObject::connect(portal,
                             &OrgFreedesktopPortalSettingsInterface::SettingChanged,
                             q,
                             [this](const QString &group, const QString &key, const QDBusVariant &value) {
                                 if (group != PORTAL_GROUP) {
                                     return;
                                 }
                                 if (key == KEY_AVAILABLE) {
                                     Q_EMIT q->tabletModeAvailableChanged(value.variant().toBool());
                                 } else if (key == KEY_ENABLED) {
                                     setIsTablet(value.variant().toBool());
                                 }
                             });

            const auto reply = portal->ReadAll({PORTAL_GROUP});
            auto watcher = new QDBusPendingCallWatcher(reply, q);
            QObject::connect(watcher, &QDBusPendingCallWatcher::finished, q, [this, watcher]() {
                watcher->deleteLater();
                QDBusPendingReply<VariantMapMap> reply = *watcher;
                const auto properties = reply.value().value(PORTAL_GROUP);
                Q_EMIT q->tabletModeAvailableChanged(properties[KEY_AVAILABLE].toBool());
                setIsTablet(properties[KEY_ENABLED].toBool());
            });
        }
// TODO: case for Windows
#endif
    }
    ~TabletModeWatcherPrivate(){};
    void setIsTablet(bool tablet);

    TabletModeWatcher *q;
    QList<QObject *> watchers;
    bool isTabletModeAvailable = false;
    bool isTabletMode = false;
};

void TabletModeWatcherPrivate::setIsTablet(bool tablet)
{
    if (isTabletMode == tablet) {
        return;
    }

    isTabletMode = tablet;
    TabletModeChangedEvent event{tablet};
    Q_EMIT q->tabletModeChanged(tablet);
    for (auto *w : watchers) {
        QCoreApplication::sendEvent(w, &event);
    }
}

TabletModeWatcher::TabletModeWatcher(QObject *parent)
    : QObject(parent)
    , d(new TabletModeWatcherPrivate(this))
{
}

TabletModeWatcher::~TabletModeWatcher()
{
    delete d;
}

TabletModeWatcher *TabletModeWatcher::self()
{
    return &privateTabletModeWatcherSelf()->self;
}

bool TabletModeWatcher::isTabletModeAvailable() const
{
    return d->isTabletModeAvailable;
}

bool TabletModeWatcher::isTabletMode() const
{
    return d->isTabletMode;
}

void TabletModeWatcher::addWatcher(QObject *watcher)
{
    d->watchers.append(watcher);
}

void TabletModeWatcher::removeWatcher(QObject *watcher)
{
    d->watchers.removeAll(watcher);
}

}
}

#include "moc_tabletmodewatcher.cpp"
