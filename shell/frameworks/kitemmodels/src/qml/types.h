/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQmlEngine>

#include <KColumnHeadersModel>
#include <KNumberModel>

struct KColumnHeadersModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(KColumnHeadersModel)
    QML_FOREIGN(KColumnHeadersModel)
};

struct KNumberModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(KNumberModel)
    QML_FOREIGN(KNumberModel)
};
