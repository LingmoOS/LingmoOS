// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusArgument>
#include <QMap>
#include <QString>

/// a{sa{sv}}
using VariantMapMap = QMap<QString, QMap<QString, QVariant>>;

Q_DECLARE_METATYPE(VariantMapMap)
