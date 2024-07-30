/*
    SPDX-FileCopyrightText: 2007-2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGLOADER_P_H
#define KCONFIGLOADER_P_H

#include <QUrl>

class ConfigLoaderPrivate
{
public:
    ConfigLoaderPrivate()
        : saveDefaults(false)
    {
    }

    ~ConfigLoaderPrivate()
    {
        clearData();
    }

    void clearData()
    {
        qDeleteAll(bools);
        qDeleteAll(strings);
        qDeleteAll(stringlists);
        qDeleteAll(colors);
        qDeleteAll(fonts);
        qDeleteAll(ints);
        qDeleteAll(uints);
        qDeleteAll(urls);
        qDeleteAll(dateTimes);
        qDeleteAll(doubles);
        qDeleteAll(intlists);
        qDeleteAll(longlongs);
        qDeleteAll(points);
        qDeleteAll(pointfs);
        qDeleteAll(rects);
        qDeleteAll(rectfs);
        qDeleteAll(sizes);
        qDeleteAll(sizefs);
        qDeleteAll(ulonglongs);
        qDeleteAll(urllists);
    }

    bool *newBool()
    {
        bool *v = new bool;
        bools.append(v);
        return v;
    }

    QString *newString()
    {
        QString *v = new QString;
        strings.append(v);
        return v;
    }

    QStringList *newStringList()
    {
        QStringList *v = new QStringList;
        stringlists.append(v);
        return v;
    }

    QColor *newColor()
    {
        QColor *v = new QColor;
        colors.append(v);
        return v;
    }

    QFont *newFont()
    {
        QFont *v = new QFont;
        fonts.append(v);
        return v;
    }

    qint32 *newInt()
    {
        qint32 *v = new qint32;
        ints.append(v);
        return v;
    }

    quint32 *newUint()
    {
        quint32 *v = new quint32;
        uints.append(v);
        return v;
    }

    QUrl *newUrl()
    {
        QUrl *v = new QUrl;
        urls.append(v);
        return v;
    }

    QDateTime *newDateTime()
    {
        QDateTime *v = new QDateTime;
        dateTimes.append(v);
        return v;
    }

    double *newDouble()
    {
        double *v = new double;
        doubles.append(v);
        return v;
    }

    QList<qint32> *newIntList()
    {
        QList<qint32> *v = new QList<qint32>;
        intlists.append(v);
        return v;
    }

    qint64 *newLongLong()
    {
        qint64 *v = new qint64;
        longlongs.append(v);
        return v;
    }

    QPoint *newPoint()
    {
        QPoint *v = new QPoint;
        points.append(v);
        return v;
    }

    QPointF *newPointF()
    {
        QPointF *v = new QPointF;
        pointfs.append(v);
        return v;
    }

    QRect *newRect()
    {
        QRect *v = new QRect;
        rects.append(v);
        return v;
    }

    QRectF *newRectF()
    {
        QRectF *v = new QRectF;
        rectfs.append(v);
        return v;
    }

    QSize *newSize()
    {
        QSize *v = new QSize;
        sizes.append(v);
        return v;
    }

    QSizeF *newSizeF()
    {
        QSizeF *v = new QSizeF;
        sizefs.append(v);
        return v;
    }

    quint64 *newULongLong()
    {
        quint64 *v = new quint64;
        ulonglongs.append(v);
        return v;
    }

    QList<QUrl> *newUrlList()
    {
        QList<QUrl> *v = new QList<QUrl>();
        urllists.append(v);
        return v;
    }

    void parse(KConfigLoader *loader, QIODevice *xml);

    /**
     * Whether or not to write out default values.
     *
     * @param writeDefaults true if defaults should be written out
     */
    void setWriteDefaults(bool writeDefaults)
    {
        saveDefaults = writeDefaults;
    }

    /**
     * @return true if default values will also be written out
     */
    bool writeDefaults() const
    {
        return saveDefaults;
    }

    QList<bool *> bools;
    QList<QString *> strings;
    QList<QStringList *> stringlists;
    QList<QColor *> colors;
    QList<QFont *> fonts;
    QList<qint32 *> ints;
    QList<quint32 *> uints;
    QList<QUrl *> urls;
    QList<QDateTime *> dateTimes;
    QList<double *> doubles;
    QList<QList<qint32> *> intlists;
    QList<qint64 *> longlongs;
    QList<QPoint *> points;
    QList<QPointF *> pointfs;
    QList<QRect *> rects;
    QList<QRectF *> rectfs;
    QList<QSize *> sizes;
    QList<QSizeF *> sizefs;
    QList<quint64 *> ulonglongs;
    QList<QList<QUrl> *> urllists;
    QString baseGroup;
    QStringList groups;
    QHash<QString, QString> keysToNames;
    bool saveDefaults;
};

#endif
