// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Use qdbusxml2cpp panels/dock/api/old/org.deepin.dde.daemon.Dock1.xml -p dockrect

#ifndef DOCKRECT_H
#define DOCKRECT_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.deepin.dde.daemon.Dock1
 */
class OrgDeepinDdeDaemonDock1Interface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "org.deepin.dde.daemon.Dock1";
    }

public:
    OrgDeepinDdeDaemonDock1Interface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~OrgDeepinDdeDaemonDock1Interface();

    Q_PROPERTY(int DisplayMode READ displayMode WRITE setDisplayMode)
    inline int displayMode() const
    {
        return qvariant_cast<int>(property("DisplayMode"));
    }
    inline void setDisplayMode(int value)
    {
        setProperty("DisplayMode", QVariant::fromValue(value));
    }

    Q_PROPERTY(QRect FrontendWindowRect READ frontendWindowRect)
    inline QRect frontendWindowRect() const
    {
        return qvariant_cast<QRect>(property("FrontendWindowRect"));
    }

    Q_PROPERTY(int HideMode READ hideMode WRITE setHideMode)
    inline int hideMode() const
    {
        return qvariant_cast<int>(property("HideMode"));
    }
    inline void setHideMode(int value)
    {
        setProperty("HideMode", QVariant::fromValue(value));
    }

    Q_PROPERTY(int HideState READ hideState)
    inline int hideState() const
    {
        return qvariant_cast<int>(property("HideState"));
    }

    Q_PROPERTY(int Position READ position WRITE setPosition)
    inline int position() const
    {
        return qvariant_cast<int>(property("Position"));
    }
    inline void setPosition(int value)
    {
        setProperty("Position", QVariant::fromValue(value));
    }

    Q_PROPERTY(uint WindowSizeEfficient READ windowSizeEfficient WRITE setWindowSizeEfficient)
    inline uint windowSizeEfficient() const
    {
        return qvariant_cast<uint>(property("WindowSizeEfficient"));
    }
    inline void setWindowSizeEfficient(uint value)
    {
        setProperty("WindowSizeEfficient", QVariant::fromValue(value));
    }

    Q_PROPERTY(uint WindowSizeFashion READ windowSizeFashion WRITE setWindowSizeFashion)
    inline uint windowSizeFashion() const
    {
        return qvariant_cast<uint>(property("WindowSizeFashion"));
    }
    inline void setWindowSizeFashion(uint value)
    {
        setProperty("WindowSizeFashion", QVariant::fromValue(value));
    }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<bool> IsDocked(const QString &desktopFile)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(desktopFile);
        return asyncCallWithArgumentList(QStringLiteral("IsDocked"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestDock(const QString &desktopFile, int index)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(desktopFile) << QVariant::fromValue(index);
        return asyncCallWithArgumentList(QStringLiteral("RequestDock"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestUndock(const QString &desktopFile)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(desktopFile);
        return asyncCallWithArgumentList(QStringLiteral("RequestUndock"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void DisplayModeChanged(int displaymode);
    void FrontendWindowRectChanged(const QRect &FrontendWindowRect);
    void HideModeChanged(int hideMode);
    void PositionChanged(int Position);
    void WindowSizeEfficientChanged(uint size);
    void WindowSizeFashionChanged(uint size);
};

namespace org
{
namespace deepin
{
namespace dde
{
namespace daemon
{
typedef ::OrgDeepinDdeDaemonDock1Interface Dock1;
}
}
}
}
#endif
