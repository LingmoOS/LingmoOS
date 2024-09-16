// SPDX-FileCopyrightText: 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qminimalbackingstore.h"
#include "qminimalintegration.h"
#include "qscreen.h"
#include <QtCore/qdebug.h>
#include <qpa/qplatformscreen.h>
#include <private/qguiapplication_p.h>

MinimalBackingStore::MinimalBackingStore(QWindow *window)
    : QPlatformBackingStore(window)
{
}

MinimalBackingStore::~MinimalBackingStore()
{
}

QPaintDevice *MinimalBackingStore::paintDevice()
{
    return &mImage;
}

void MinimalBackingStore::flush(QWindow *, const QRegion &, const QPoint &)
{
}

void MinimalBackingStore::resize(const QSize &size, const QRegion &)
{
    QImage::Format format = QGuiApplication::primaryScreen()->handle()->format();
    if (mImage.size() != size)
        mImage = QImage(size, format);
}
