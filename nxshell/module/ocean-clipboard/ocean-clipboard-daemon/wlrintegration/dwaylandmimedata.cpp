// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dwaylandmimedata.h"
#include <QImageReader>

static const QString ApplicationXQtImageLiteral QStringLiteral("application/x-qt-image");

static QStringList imageMimeFormats(const QList<QByteArray> &imageFormats)
{
    QStringList formats;
    formats.reserve(imageFormats.size());
    for (const auto &format : imageFormats)
        formats.append(QLatin1String("image/") + QLatin1String(format.toLower()));

    // put png at the front because it is best
    int pngIndex = formats.indexOf(QLatin1String("image/png"));
    if (pngIndex != -1 && pngIndex != 0)
        formats.move(pngIndex, 0);

    return formats;
}

static inline QStringList imageReadMimeFormats()
{
    return imageMimeFormats(QImageReader::supportedImageFormats());
}

DWaylandMimeData::DWaylandMimeData()
{

}

DWaylandMimeData::~DWaylandMimeData()
{

}

QStringList DWaylandMimeData::formats() const
{
    QStringList realFormats = QMimeData::formats();
    if (!realFormats.contains(QLatin1String("application/x-qt-image"))) {
        QStringList imageFormats = imageReadMimeFormats();
        for (int i = 0; i < imageFormats.size(); ++i) {
            if (realFormats.contains(imageFormats.at(i))) {
                realFormats += QLatin1String("application/x-qt-image");
                break;
            }
        }
    }
    return realFormats;
}

QVariant DWaylandMimeData::retrieveData(const QString &mimeType, QMetaType preferredType) const
{
    QVariant data = QMimeData::retrieveData(mimeType, preferredType);
    if (mimeType == QLatin1String("application/x-qt-image")) {
        if (data.isNull() || (data.userType() == QMetaType::QByteArray && data.toByteArray().isEmpty())) {
            // try to find an image
            QStringList imageFormats = imageReadMimeFormats();
            for (int i = 0; i < imageFormats.size(); ++i) {
                data = QMimeData::retrieveData(imageFormats.at(i), preferredType);
                if (data.isNull() || (data.userType() == QMetaType::QByteArray && data.toByteArray().isEmpty()))
                    continue;
                break;
            }
        }
        int typeId = preferredType.id();
        Q_ASSERT(typeId != QMetaType::UnknownType);
        // we wanted some image type, but all we got was a byte array. Convert it to an image.
        if (data.userType() == QMetaType::QByteArray
            && (typeId == QMetaType::QImage || typeId == QMetaType::QPixmap || typeId == QMetaType::QBitmap))
            data = QImage::fromData(data.toByteArray());
    } else if (mimeType == QLatin1String("application/x-color") && data.userType() == QMetaType::QByteArray) {
        QColor c;
        QByteArray ba = data.toByteArray();
        if (ba.size() == 8) {
            ushort * colBuf = (ushort *)ba.data();
            c.setRgbF(qreal(colBuf[0]) / qreal(0xFFFF),
                      qreal(colBuf[1]) / qreal(0xFFFF),
                      qreal(colBuf[2]) / qreal(0xFFFF),
                      qreal(colBuf[3]) / qreal(0xFFFF));
            data = c;
        } else {
            qWarning() << "Qt: Invalid color format";
        }
    }
    return data;
}
