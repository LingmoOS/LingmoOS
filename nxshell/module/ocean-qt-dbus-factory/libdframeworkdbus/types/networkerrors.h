// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKERRORS_H
#define NETWORKERRORS_H

#include <QList>
#include <QVariant>
#include <QDBusMetaType>

typedef QMap<QString, QMap<QString, QString>> NetworkErrors;

void registerNetworkErrorsMetaType();

#endif // NETWORKERRORS_H
