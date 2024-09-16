// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawinterface.h"
#include <QDBusMetaType>

DrawInterface::DrawInterface(const QString &serviceName, const QString &ObjectPath,
                             const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(serviceName, ObjectPath, staticInterfaceName(), connection, parent)
{

}

DrawInterface::~DrawInterface()
{

}
