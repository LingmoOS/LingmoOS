// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageprovider.h"

#include <QScreen>
#include <QGuiApplication>

ImageProvider::ImageProvider()
    : QQuickImageProvider(Pixmap)
{

}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize)

    if (id.startsWith("screen/")) {
        const QString &screen_name = id.mid(7);

        for (QScreen *s : qApp->screens()) {
            if (s->name() == screen_name) {
                const QPixmap &pix = s->grabWindow(0);

                if (size)
                    *size = pix.size();

                return pix;
            }
        }
    }

    return QPixmap();
}
