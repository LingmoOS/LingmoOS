/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCMUTILS_QML_TYPES
#define KCMUTILS_QML_TYPES

#include <QQmlEngine>

#include <kquickconfigmodule.h>

struct ConfigModuleForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(ConfigModule)
    QML_UNCREATABLE("")
    QML_FOREIGN(KQuickConfigModule)
};

#endif
