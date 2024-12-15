// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorpickerinterface.h"

/*
 * Implementation of interface class ColorPickerInterface
 */

ColorPickerInterface::ColorPickerInterface(const QString &service,
                                           const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ColorPickerInterface::~ColorPickerInterface()
{
}

