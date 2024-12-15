// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "osddbusadaptor.h"
#include "osdpanel.h"

namespace osd {

OsdDBusAdaptor::OsdDBusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

void OsdDBusAdaptor::ShowOSD(const QString &text)
{
    return impl()->ShowOSD(text);
}

OsdPanel *OsdDBusAdaptor::impl() const
{
    return qobject_cast<OsdPanel *>(parent());
}

} // osd
