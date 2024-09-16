// SPDX-FileCopyrightText: 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QBACKINGSTORE_MINIMAL_H
#define QBACKINGSTORE_MINIMAL_H

#include <qpa/qplatformbackingstore.h>
#include <qpa/qplatformwindow.h>
#include <QtGui/QImage>

class MinimalBackingStore : public QPlatformBackingStore
{
public:
    MinimalBackingStore(QWindow *window);
    ~MinimalBackingStore() override;

    QPaintDevice *paintDevice() override;
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;
    void resize(const QSize &size, const QRegion &staticContents) override;

private:
    QImage mImage;
};

#endif
