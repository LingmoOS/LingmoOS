/*
    SPDX-FileCopyrightText: 2008 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2009 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpixmapsequence.h"

#include "loggingcategory.h"

#include <QList>
#include <QPixmap>

class KPixmapSequencePrivate : public QSharedData
{
public:
    QList<QPixmap> mFrames;

    void loadSequence(const QPixmap &bigPixmap, const QSize &frameSize);
};

void KPixmapSequencePrivate::loadSequence(const QPixmap &bigPixmap, const QSize &frameSize)
{
    if (bigPixmap.isNull()) {
        qCWarning(KWidgetsAddonsLog) << "Invalid pixmap specified.";
        return;
    }

    QSize size(frameSize);
    if (!size.isValid()) {
        size = QSize(bigPixmap.width(), bigPixmap.width());
    }
    if (bigPixmap.width() % size.width() || bigPixmap.height() % size.height()) {
        qCWarning(KWidgetsAddonsLog) << "Invalid framesize.";
        return;
    }

    const int rowCount = bigPixmap.height() / size.height();
    const int colCount = bigPixmap.width() / size.width();
    mFrames.resize(rowCount * colCount);

    int pos = 0;
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            QPixmap pix = bigPixmap.copy(col * size.width(), row * size.height(), size.width(), size.height());
            mFrames[pos++] = pix;
        }
    }
}

KPixmapSequence::KPixmapSequence()
    : d(new KPixmapSequencePrivate)
{
}

KPixmapSequence::KPixmapSequence(const KPixmapSequence &other)
{
    d = other.d;
}

KPixmapSequence::KPixmapSequence(const QPixmap &bigPixmap, const QSize &frameSize)
    : d(new KPixmapSequencePrivate)
{
    d->loadSequence(bigPixmap, frameSize);
}

KPixmapSequence::KPixmapSequence(const QString &fullPath, int size)
    : d(new KPixmapSequencePrivate)
{
    d->loadSequence(QPixmap(fullPath), QSize(size, size));
}

KPixmapSequence::~KPixmapSequence()
{
}

KPixmapSequence &KPixmapSequence::operator=(const KPixmapSequence &other)
{
    d = other.d;
    return *this;
}

bool KPixmapSequence::isValid() const
{
    return !isEmpty();
}

bool KPixmapSequence::isEmpty() const
{
    return d->mFrames.isEmpty();
}

QSize KPixmapSequence::frameSize() const
{
    if (isEmpty()) {
        qCWarning(KWidgetsAddonsLog) << "No frame loaded";
        return QSize();
    }
    return d->mFrames[0].size();
}

int KPixmapSequence::frameCount() const
{
    return d->mFrames.size();
}

QPixmap KPixmapSequence::frameAt(int index) const
{
    if (isEmpty() || index > frameCount() - 1) {
        qCWarning(KWidgetsAddonsLog) << "No frame loaded";
        return QPixmap();
    }
    return d->mFrames.at(index);
}
