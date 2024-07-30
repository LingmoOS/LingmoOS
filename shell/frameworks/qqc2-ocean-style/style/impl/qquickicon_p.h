/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
 */
// Almost completely copy/pasted from qtquickcontrols2 so that I don't need to depend on private headers

#ifndef QQUICKICON_P_H
#define QQUICKICON_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include <QtGui/qcolor.h>
#include <QtQml/qqml.h>

namespace Ocean
{

class QQuickIconPrivate;

class QQuickIcon
{
    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE setName RESET resetName FINAL)
    Q_PROPERTY(QUrl source READ source WRITE setSource RESET resetSource FINAL)
    Q_PROPERTY(int width READ width WRITE setWidth RESET resetWidth FINAL)
    Q_PROPERTY(int height READ height WRITE setHeight RESET resetHeight FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor FINAL)
    Q_PROPERTY(bool cache READ cache WRITE setCache RESET resetCache FINAL)

public:
    QQuickIcon();
    QQuickIcon(const QQuickIcon &other);
    ~QQuickIcon();

    QQuickIcon &operator=(const QQuickIcon &other);
    bool operator==(const QQuickIcon &other) const;
    bool operator!=(const QQuickIcon &other) const;

    bool isEmpty() const;

    QString name() const;
    void setName(const QString &name);
    void resetName();

    QUrl source() const;
    void setSource(const QUrl &source);
    void resetSource();

    QString nameOrSource() const;

    int width() const;
    void setWidth(int width);
    void resetWidth();

    int height() const;
    void setHeight(int height);
    void resetHeight();

    QColor color() const;
    void setColor(const QColor &color);
    void resetColor();

    bool cache() const;
    void setCache(bool cache);
    void resetCache();

    QQuickIcon resolve(const QQuickIcon &other) const;

private:
    QExplicitlySharedDataPointer<QQuickIconPrivate> d;
};

}

#endif // QQUICKICON_P_H
