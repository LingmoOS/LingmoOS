// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lingmoid_interface.h"

/*
 * Implementation of interface class LingmoIDInterface
 */

LingmoIDInterface::LingmoIDInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

LingmoIDInterface::~LingmoIDInterface()
{
}

