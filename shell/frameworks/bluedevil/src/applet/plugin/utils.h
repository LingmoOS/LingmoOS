/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <BluezQt/DevicesModel>
#include <QSortFilterProxyModel>

#include <qqmlregistration.h>

class Utils : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    using QObject::QObject;

    Q_INVOKABLE static QString adapterHciString(const QString &ubi);
};
