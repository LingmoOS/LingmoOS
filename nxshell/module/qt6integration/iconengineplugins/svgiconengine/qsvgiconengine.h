// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QSVGICONENGINE_H
#define QSVGICONENGINE_H

#include <QtGui/qiconengine.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QSvgIconEnginePrivate;

class QSvgIconEngine : public QIconEngine
{
public:
    QSvgIconEngine();
    QSvgIconEngine(const QSvgIconEngine &other);
    ~QSvgIconEngine();
    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state);
    QSize actualSize(const QSize &size, QIcon::Mode mode,
                     QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state);

    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                   QIcon::State state);
    void addFile(const QString &fileName, const QSize &size,
                 QIcon::Mode mode, QIcon::State state);

    QString key() const;
    QIconEngine *clone() const;
    bool read(QDataStream &in);
    bool write(QDataStream &out) const;

private:
    QSharedDataPointer<QSvgIconEnginePrivate> d;
};

QT_END_NAMESPACE

#endif
