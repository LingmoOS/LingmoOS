// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wldpms_manager_interface.h"
#include <QtDBus/QDBusMessage>
#include <QDBusInterface>


WlDpmsManagerInterface::WlDpmsManagerInterface(DpmsManager *manager)
    : QDBusAbstractAdaptor(manager)
{
    setAutoRelaySignals(true);
}

WlDpmsManagerInterface::~WlDpmsManagerInterface()
{

}

void WlDpmsManagerInterface::dpmsDbusAdd(const QString& dbus)
{
    m_dpmsDbusPath.append(dbus);
}

void WlDpmsManagerInterface::dpmsDbusRemove(const QString& dbus)
{
    m_dpmsDbusPath.removeOne(dbus);
}

QList<QVariant> WlDpmsManagerInterface::dpmsList() const
{
    return  m_dpmsDbusPath;
}
