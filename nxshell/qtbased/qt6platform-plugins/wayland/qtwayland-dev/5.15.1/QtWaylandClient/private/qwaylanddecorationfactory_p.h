// Copyright (C) 2016 Robin Burchell <robin.burchell@viroteck.net>
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDDECORATIONFACTORY_H
#define QWAYLANDDECORATIONFACTORY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandAbstractDecoration;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDecorationFactory
{
public:
    static QStringList keys(const QString &pluginPath = QString());
    static QWaylandAbstractDecoration *create(const QString &name, const QStringList &args, const QString &pluginPath = QString());
};

}

QT_END_NAMESPACE

#endif // QWAYLANDDECORATIONFACTORY_H
