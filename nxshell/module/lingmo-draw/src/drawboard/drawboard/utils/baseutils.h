// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QCursor>
#include <QFont>
#include <QLayout>
#include <QFontMetrics>
#include <QPainter>


//#include <dscreenwindowsutil.h>

//DWM_USE_NAMESPACE


enum CutRation {
    Ration4_3,
    Ration8_5,
    Ration16_9,
    Ration1_1,
    FreeRation,
};


QList<QColor> specifiedColorList();
QList<QColor> specifiedColorList2();
//QColor       colorIndexOf(int index);
//int                colorIndex(QColor color);

//QSize      initArtboardSize(QPoint pos);
//QSize      getCanvasSize(QSize artboardSize, QSize windowSize);

//QCursor setCursorShape(QString cursorName);
int stringWidth(const QFont &f, const QString &str);
//QString     getFileContent(const QString &file);
//bool          isValidFormat(QString suffix);
//bool          isCommandExist(QString command);
//void  paintSelectedPoint(QPainter &painter, QPointF pos,
//QPixmap pointImg);
//QVariantList cachePixmap(const QString &path);
//QString DetectImageFormat(const QString &filepath);
//QString allImageformat();
//QString     createHash(const QString &str);
//QString sizeToHuman(const qlonglong bytes);
//QCursor blurToolCursor(const int &lineWidth);
QCursor pickColorCursor();


#endif // BASEUTILS_H
