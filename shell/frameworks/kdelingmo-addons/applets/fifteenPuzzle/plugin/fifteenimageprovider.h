/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef FIFTEENIMAGEPROVIDER_H
#define FIFTEENIMAGEPROVIDER_H

#include <QList>
#include <QQuickImageProvider>
#include <QString>

class FifteenImageProvider : public QQuickImageProvider
{
public:
    FifteenImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    // Update our pixmaps, called when sizes change or boardSize changes
    void updatePixmaps();

    QString m_imagePath;
    QPixmap m_pixmap;
    int m_boardSize;
    int m_pieceWidth;
    int m_pieceHeight;

    QList<QPixmap> m_pixmaps;
};

#endif
