// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dicon.h"
#include <QGuiApplication>
#include <private/qicon_p.h>
#include <QImageReader>

DGUI_BEGIN_NAMESPACE
DIcon::DIcon(const QIcon &other)
    : QIcon(other)
{

}

DIcon::~DIcon()
{

}

/*! Icon::pixmap
 * Returns a pixmap with the requested \a size, \a devicePixelRatio, \a mode, and \a state, generating one if necessary.
 *
 * \sa QIcon::actualsize(), QIcon::paint()
 */
QPixmap DIcon::pixmap(const QSize &size, qreal devicePixelRatio, QIcon::Mode mode, QIcon::State state)
{
    DataPtr d = data_ptr();
    if (!d)
        return QPixmap();

    if (qFuzzyCompare(devicePixelRatio, -1))
        devicePixelRatio = qApp->devicePixelRatio();

    if (!(devicePixelRatio > 1.0)) {
        QPixmap pixmap = d->engine->pixmap(size, mode, state);
        pixmap.setDevicePixelRatio(1.0);
        return pixmap;
    }

    QPixmap pixmap = d->engine->scaledPixmap(size * devicePixelRatio, mode, state, devicePixelRatio);

    auto pixmapDevicePixelRatio = [](qreal displayDevicePixelRatio,
            const QSize &requestedSize, const QSize &actualSize)->qreal {
        QSize targetSize = requestedSize * displayDevicePixelRatio;
        if ((actualSize.width() == targetSize.width() && actualSize.height() <= targetSize.height()) ||
                (actualSize.width() <= targetSize.width() && actualSize.height() == targetSize.height())) {
            return displayDevicePixelRatio;
        }
        qreal scale = 0.5 * (qreal(actualSize.width()) / qreal(targetSize.width()) +
                             qreal(actualSize.height() / qreal(targetSize.height())));
        return qMax(qreal(1.0), displayDevicePixelRatio * scale);
    };

    pixmap.setDevicePixelRatio(pixmapDevicePixelRatio(devicePixelRatio, size, pixmap.size()));
    return pixmap;
}

/*!
 * @brief DIcon::loadNxPixmap loads the suitable @Nx image.
 * @param fileName is the original resource file name.
 * @return the hiDPI ready QPixmap.
 */
QPixmap DIcon::loadNxPixmap(const QString &fileName)
{
    qreal sourceDevicePixelRatio = 1.0;
    qreal devicePixelRatio = qApp->devicePixelRatio();
    QPixmap pixmap;

    if (!qFuzzyCompare(sourceDevicePixelRatio, devicePixelRatio)) {
        QImageReader reader;
        reader.setFileName(qt_findAtNxFile(fileName, devicePixelRatio, &sourceDevicePixelRatio));
        if (reader.canRead()) {
            reader.setScaledSize(reader.size() * (devicePixelRatio / sourceDevicePixelRatio));
            pixmap = QPixmap::fromImage(reader.read());
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
    } else {
        pixmap.load(fileName);
    }

    return pixmap;
}
DGUI_END_NAMESPACE
