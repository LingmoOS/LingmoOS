// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include <QApplication>
#include <QDBusInterface>
#include <QDebug>
#include <QDir>
#include <QFontMetrics>
#include <QPainter>
#include <QString>
#include <QWidget>
#include <QtMath>

static const QString WarningDialogService = "com.deepin.dde.WarningDialog";
static const QString WarningDialogPath = "/com/deepin/dde/WarningDialog";
static const QString WarningDialogInterface = "com.deepin.dde.WarningDialog";

QString Utils::getQrcPath(QString imageName)
{
    return QString(":/image/%1").arg(imageName);
}

//QString Utils::getQssPath(QString qssName)
//{
//    return QString(":/qss/%1").arg(qssName);
//}

QString Utils::colorToHex(QColor color)
{
    return color.name().toUpper();
}

QString Utils::colorToCMYK(QColor color)
{
    return QString("(%1, %2, %3, %4)").arg(color.cyan()).arg(color.magenta()).arg(color.yellow()).arg(color.black());
}

QString Utils::colorToHSV(QColor color)
{
    return QString("(%1, %2, %3)").arg(color.hue()).arg(color.saturation()).arg(color.value());
}

QString Utils::colorToRGB(QColor color)
{
    return QString("(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
}

QString Utils::colorToRGBA(QColor color)
{
    return QString("(%1, %2, %3, 1.0)").arg(color.red()).arg(color.green()).arg(color.blue());
}

QString Utils::colorToFloatRGB(QColor color)
{
    return QString("(%1, %2, %3)").arg(color.red() / 255.0).arg(color.green() / 255.0).arg(color.blue() / 255.0);
}

QString Utils::colorToFloatRGBA(QColor color)
{
    return QString("(%1, %2, %3, 1.0)").arg(color.red() / 255.0).arg(color.green() / 255.0).arg(color.blue() / 255.0);
}

qreal Utils::easeInOut(qreal x)
{
    return (1 - qCos(M_PI * x)) / 2;
}

//qreal Utils::easeInQuad(qreal x)
//{
//    return qPow(x, 2);
//}

//qreal Utils::easeOutQuad(qreal x)
//{
//    return -1 * qPow(x - 1, 2) + 1;
//}

//qreal Utils::easeInQuint(qreal x)
//{
//    return qPow(x, 5);
//}

//qreal Utils::easeOutQuint(qreal x)
//{
//    return qPow(x - 1, 5) + 1;
//}

//void Utils::warnNoComposite()
//{
//    QDBusInterface iface(WarningDialogService,
//                         WarningDialogPath,
//                         WarningDialogService);
//    iface.call("RaiseWindow");
//}

