/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCMUTILS_PRIVATE_QML_TYPES
#define KCMUTILS_PRIVATE_QML_TYPES

#include <QQmlEngine>

#include <kpluginproxymodel.h>

struct PluginProxyModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(ProxyModel)
    QML_FOREIGN(KPluginProxyModel)
};

#endif
