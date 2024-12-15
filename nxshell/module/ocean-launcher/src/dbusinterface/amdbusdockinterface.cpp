// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "amdbusdockinterface.h"

class DockPrivate
{
public:
   DockPrivate() = default;

    // begin member variables
    int DisplayMode;
    QStringList DockedApps;
    QList<QDBusObjectPath> Entries;
    QRect FrontendWindowRect;
    int HideMode;
    int HideState;
    uint HideTimeout;
    uint IconSize;
    double Opacity;
    int Position;
    uint ShowTimeout;
    uint WindowSize;
    uint WindowSizeEfficient;
    uint WindowSizeFashion;

public:
    QMap<QString, QDBusPendingCallWatcher *> m_processingCalls;
    QMap<QString, QList<QVariant>> m_waittingCalls;
};

AMDBusDockInter::AMDBusDockInter(QObject *parent)
    : QDBusAbstractInterface(INTERFACE_NAME, SERVICE_PATH, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
    , d_ptr(new DockPrivate)
{
    QDBusConnection::sessionBus().connect(service(), path(), "org.freedesktop.DBus.Properties", "PropertiesChanged","sa{sv}as", this, SLOT(onPropertyChanged(const QDBusMessage &)));
}

AMDBusDockInter::~AMDBusDockInter()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties", "PropertiesChanged","sa{sv}as", this, SLOT(onPropertyChanged(const QDBusMessage &)));
    qDeleteAll(d_ptr->m_processingCalls.values());
    delete d_ptr;
}

void AMDBusDockInter::onPropertyChanged(const QDBusMessage& msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count())
        return;

    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != staticInterfaceName())
        return;

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach(const QString &prop, keys) {
        const QMetaObject *self = metaObject();
        for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
            QMetaProperty property = self->property(i);
            if (property.name() == prop)
                Q_EMIT property.notifySignal().invoke(this);
        }
    }
}

int AMDBusDockInter::displayMode()
{
    return qvariant_cast<int>(property("DisplayMode"));
}

void AMDBusDockInter::setDisplayMode(int value)
{
    setProperty("DisplayMode", QVariant::fromValue(value));
}

QStringList AMDBusDockInter::dockedApps()
{
    return qvariant_cast<QStringList>(property("DockedApps"));
}

QList<QDBusObjectPath> AMDBusDockInter::entries()
{
    return qvariant_cast<QList<QDBusObjectPath>>(property("Entries"));
}

QRect AMDBusDockInter::frontendWindowRect()
{
    return qvariant_cast<QRect>(property("FrontendWindowRect"));
}

int AMDBusDockInter::hideMode()
{
    return qvariant_cast<int>(property("HideMode"));
}

void AMDBusDockInter::setHideMode(int value)
{

   internalPropSet("HideMode", QVariant::fromValue(value));
}

int AMDBusDockInter::hideState()
{
    return qvariant_cast<int>(property("HideState"));
}

uint AMDBusDockInter::hideTimeout()
{
    return qvariant_cast<uint>(property("HideTimeout"));
}

void AMDBusDockInter::setHideTimeout(uint value)
{

   setProperty("HideTimeout", QVariant::fromValue(value));
}

uint AMDBusDockInter::iconSize()
{
    return qvariant_cast<uint>(property("IconSize"));
}

void AMDBusDockInter::setIconSize(uint value)
{

   setProperty("IconSize", QVariant::fromValue(value));
}

double AMDBusDockInter::opacity()
{
    return qvariant_cast<double>(property("Opacity"));
}

void AMDBusDockInter::setOpacity(double value)
{

   setProperty("Opacity", QVariant::fromValue(value));
}

int AMDBusDockInter::position()
{
    return qvariant_cast<int>(property("Position"));
}

void AMDBusDockInter::setPosition(int value)
{
   setProperty("Position", QVariant::fromValue(value));
}

uint AMDBusDockInter::showTimeout()
{
    return qvariant_cast<uint>(property("ShowTimeout"));
}

void AMDBusDockInter::setShowTimeout(uint value)
{

   setProperty("ShowTimeout", QVariant::fromValue(value));
}

uint AMDBusDockInter::windowSize()
{
    return qvariant_cast<uint>(property("WindowSize"));
}

void AMDBusDockInter::setWindowSize(uint value)
{
   setProperty("WindowSize", QVariant::fromValue(value));
}

uint AMDBusDockInter::windowSizeEfficient()
{
    return qvariant_cast<uint>(property("WindowSizeEfficient"));
}

void AMDBusDockInter::setWindowSizeEfficient(uint value)
{

   setProperty("WindowSizeEfficient", QVariant::fromValue(value));
}

uint AMDBusDockInter::windowSizeFashion()
{
    return qvariant_cast<uint>(property("WindowSizeFashion"));
}

void AMDBusDockInter::setWindowSizeFashion(uint value)
{
   setProperty("WindowSizeFashion", QVariant::fromValue(value));
}

void AMDBusDockInter::CallQueued(const QString &callName, const QList<QVariant> &args)
{
    if (d_ptr->m_waittingCalls.contains(callName)) {
        d_ptr->m_waittingCalls[callName] = args;
        return;
    }

    if (d_ptr->m_processingCalls.contains(callName)) {
        d_ptr->m_waittingCalls.insert(callName, args);
    } else {
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncCallWithArgumentList(callName, args));
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &AMDBusDockInter::onPendingCallFinished);
        d_ptr->m_processingCalls.insert(callName, watcher);
    }
}

void AMDBusDockInter::onPendingCallFinished(QDBusPendingCallWatcher *w)
{
    w->deleteLater();
    const auto callName = d_ptr->m_processingCalls.key(w);
    Q_ASSERT(!callName.isEmpty());
    if (callName.isEmpty())
        return;

    d_ptr->m_processingCalls.remove(callName);
    if (!d_ptr->m_waittingCalls.contains(callName))
        return;

    const auto args = d_ptr->m_waittingCalls.take(callName);
    CallQueued(callName, args);
}
