// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "heifplugin.h"

#include "heifhandler.h"

static bool isHeifFormat(const QByteArray &format)
{
    const QByteArray name = format.toLower();
    return name == "heif" || name == "heic" || name == "avif";
}

QImageIOPlugin::Capabilities HeifPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (!device)
        return isHeifFormat(format) ? Capabilities(CanRead) : Capabilities();
    if (!device->isReadable())
        return {};
    if (!format.isEmpty() && !isHeifFormat(format))
        return {};
    return HeifHandler::canRead(device) ? Capabilities(CanRead) : Capabilities();
}

QImageIOHandler *HeifPlugin::create(QIODevice *device, const QByteArray &format) const
{
    auto *handler = new HeifHandler;
    handler->setDevice(device);
    handler->setFormat(format.isEmpty() ? QByteArrayLiteral("heif") : format);
    return handler;
}
