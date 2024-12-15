// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseutils.h"

#include <QPixmap>
#include <QProcess>
#include <QLayoutItem>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QIcon>

const QSize START_SIZE = QSize(15, 15);
const QSize RECT_SIZE = QSize(22, 26);
const QSize ARROW_SIZE = QSize(24, 24);
const QSize TEXT_SIZE = QSize(11, 23);
const QSize COLORPEN_SIZE = QSize(25, 25);

QMap<QString, QCursor> BaseUtils::m_shapesCursor = QMap<QString, QCursor>();

QCursor BaseUtils::setCursorShape(QString cursorName, int colorIndex)
{

    QString cursorNameString = cursorName;
    if (cursorName == "pen") {
        cursorNameString = QString("%1%2").arg(cursorName).arg(colorIndex);
    }

    if (BaseUtils::m_shapesCursor.find(cursorNameString) != BaseUtils::m_shapesCursor.end()) {
        return BaseUtils::m_shapesCursor.value(cursorNameString);
    }





    QCursor customShape = QCursor();
    qreal ration = qApp->devicePixelRatio();

    if (cursorName == "start") {
        QPixmap startPix;
        if (ration <= 1) {
            startPix = QIcon(":/mouse_style/shape/start_mouse.svg").pixmap(START_SIZE);
            customShape = QCursor(startPix, 8, 8);
        } else {
            startPix = QIcon(":/mouse_style/shape/start_mouse@2x.svg").pixmap(START_SIZE);
            customShape = QCursor(startPix, int(8 * ration), int(8 * ration));
        }

    } else if (cursorName == "rotate") {
        QPixmap rotateCursor  = QIcon(":/mouse_style/shape/rotate_mouse.svg").pixmap(ARROW_SIZE);
        rotateCursor.setDevicePixelRatio(ration);

        customShape = QCursor(rotateCursor, int(10 * ration), int(10 * ration));
    } else if (cursorName == "rectangle") {
        QPixmap rectCursor  = QIcon(":/mouse_style/shape/rect_mouse.svg").pixmap(RECT_SIZE);
        rectCursor.setDevicePixelRatio(ration);

        customShape = QCursor(rectCursor, 0, int(1 * ration));
    } else if (cursorName == "oval") {
        QPixmap ovalCursor  = QIcon(":/mouse_style/shape/ellipse_mouse.svg").pixmap(RECT_SIZE);
        ovalCursor.setDevicePixelRatio(ration);

        customShape = QCursor(ovalCursor, 0, int(1 * ration));
    } else if (cursorName == "arrow") {
        QPixmap arrowCursor  = QIcon(":/mouse_style/shape/arrow_mouse.svg").pixmap(ARROW_SIZE);
        arrowCursor.setDevicePixelRatio(ration);

        customShape = QCursor(arrowCursor, int(5 * ration), int(5 * ration));
    } else if (cursorName == "text") {
        QPixmap textCursor  = QIcon(":/mouse_style/shape/text_mouse.svg").pixmap(TEXT_SIZE);
        textCursor.setDevicePixelRatio(ration);

        customShape = QCursor(textCursor, int(5 * ration), int(5 * ration));
    } else if (cursorName == "pen") {
        QPixmap colorPic = QIcon(QString(":/color_pen/color%1.svg").arg(colorIndex)).pixmap(COLORPEN_SIZE);
        colorPic.setDevicePixelRatio(ration);

        customShape = QCursor(colorPic,  int(5 * ration), int(22 * ration));
    } else if (cursorName == "line") {
        QPixmap lineCursor  = QIcon(":/mouse_style/shape/line_mouse.svg").pixmap(ARROW_SIZE);
        lineCursor.setDevicePixelRatio(ration);

        customShape = QCursor(lineCursor, int(2 * ration), int(9 * ration));
    }

    BaseUtils::m_shapesCursor.insert(cursorNameString, customShape);
    return customShape;
}

int BaseUtils::stringWidth(const QFont &f, const QString &str)
{
    QFontMetrics fm(f);
    return fm.boundingRect(str).width();
}

QColor BaseUtils::colorIndexOf(int index)
{
    QList<QColor> colorList;
    colorList.append(QColor("#000000"));
    colorList.append(QColor("#7D7D7D"));
    colorList.append(QColor("#FFFFFF"));
    colorList.append(QColor("#F82A2A"));
    colorList.append(QColor("#FF8100"));
    colorList.append(QColor("#FFF100"));
    colorList.append(QColor("#CDFF00"));
    colorList.append(QColor("#1EE9A8"));
    colorList.append(QColor("#006D06"));
    colorList.append(QColor("#0089F7"));
    colorList.append(QColor("#7600AC"));
    colorList.append(QColor("#0C00A0"));
    return colorList[index];
}

int BaseUtils::colorIndex(QColor color)
{
    QList<QColor> colorList;
    colorList.append(QColor("#000000"));
    colorList.append(QColor("#7D7D7D"));
    colorList.append(QColor("#FFFFFF"));
    colorList.append(QColor("#F82A2A"));
    colorList.append(QColor("#FF8100"));
    colorList.append(QColor("#FFF100"));
    colorList.append(QColor("#CDFF00"));
    colorList.append(QColor("#1EE9A8"));
    colorList.append(QColor("#006D06"));
    colorList.append(QColor("#0089F7"));
    colorList.append(QColor("#7600AC"));
    colorList.append(QColor("#0C00A0"));
    return colorList.indexOf(color);
}

bool BaseUtils::isValidFormat(QString suffix)
{
    QStringList validFormat;
    validFormat << "bmp" << "jpg" << "jpeg" << "png" << "pbm" << "pgm" << "xbm" << "xpm";
    if (validFormat.contains(suffix)) {
        return true;
    } else {
        return false;
    }
}

bool BaseUtils::isCommandExist(QString command)
{
    QProcess *proc = new QProcess;
    if (!proc) {
        return false;
    }
    QString cm = QString("which %1\n").arg(command);
    proc->start(cm);
    proc->waitForFinished(1000);
    int ret = proc->exitCode() == 0;
    delete proc;
    return ret;
}
