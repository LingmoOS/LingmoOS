// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>
#include <QString>
#include <QPainter>
#include <QColor>

class Utils : public QObject
{
    Q_OBJECT

public:
    static QString getQrcPath(QString imageName);
//    static QString getQssPath(QString qssName);
    static void passInputEvent(int wid);
    static QString colorToHex(QColor color);
    static QString colorToCMYK(QColor color);
    static QString colorToHSV(QColor color);
    static QString colorToRGB(QColor color);
    static QString colorToRGBA(QColor color);
    static QString colorToFloatRGB(QColor color);
    static QString colorToFloatRGBA(QColor color);
    static qreal easeInOut(qreal x);
//    static qreal easeInQuad(qreal x);
//    static qreal easeInQuint(qreal x);
//    static qreal easeOutQuad(qreal x);
//    static qreal easeOutQuint(qreal x);
//    static void warnNoComposite();
};
