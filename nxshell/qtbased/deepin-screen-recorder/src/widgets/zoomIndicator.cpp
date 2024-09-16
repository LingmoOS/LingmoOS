// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zoomIndicator.h"
#include "../utils/baseutils.h"
#include "../utils/tempfile.h"
#include "../utils.h"

#include <QCursor>
#include <QTextOption>
#include <QDebug>
#include <QRgb>
#include <QDBusInterface>
#include <QDBusReply>
namespace {
const QSize BACKGROUND_SIZE = QSize(59, 59);
//const int SCALE_VALUE = 4;
const int IMG_WIDTH =  12;
const int INDICATOR_WIDTH = 49;
const int CENTER_RECT_WIDTH = 12;
const int BOTTOM_RECT_HEIGHT = 14;
}
ZoomIndicator::ZoomIndicator(DWidget *parent)
    : DLabel(parent)
{

    QDBusInterface wmInterface("com.deepin.wm",
                               "/com/deepin/wm",
                               "com.deepin.wm",
                               QDBusConnection::sessionBus());
    if (!wmInterface.isValid()) {
        qWarning() << "无法获取多任务视图dbus接口！";
        m_isOpenWM = false;
    } else {
        QDBusReply<bool> reply = wmInterface.call("GetMultiTaskingStatus");
        if (!reply.isValid()) {
            qWarning() << "无法调用获取多任务视图状态的dbus方法！";
            m_isOpenWM = false;
        } else {
            m_isOpenWM = reply.value();
        }
    }
    qInfo() << "多任务视图是否打开: " << (m_isOpenWM ? "是" : "否");
    if (Utils::isWaylandMode && !m_isOpenWM) {
        m_zoomIndicatorGL = new ZoomIndicatorGL(parent);
        this->hide();
        return;
    }

    setFixedSize(BACKGROUND_SIZE);
//    setStyleSheet(getFileContent(":/resources/qss/zoomindicator.qss"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
//    QString t_zoomStyle = QString("border-image: url(:/resources/images/action/magnifier.png);");
//    this->setStyleSheet(t_zoomStyle);

    m_centerRect = QRect((BACKGROUND_SIZE.width() - CENTER_RECT_WIDTH) / 2 + 1,
                         (BACKGROUND_SIZE.width() - CENTER_RECT_WIDTH) / 2 + 1,
                         CENTER_RECT_WIDTH, CENTER_RECT_WIDTH);

    m_globalRect = QRect(0, 0, BACKGROUND_SIZE.width(), BACKGROUND_SIZE.height());
}

ZoomIndicator::~ZoomIndicator()
{
    if (Utils::isWaylandMode && !m_isOpenWM && m_zoomIndicatorGL) {
        delete  m_zoomIndicatorGL;
    }
}

void ZoomIndicator::paintEvent(QPaintEvent *)
{
//    using namespace utils;
    //通过此方式获取光标的位置实际是当前光标所在的屏幕的位置加上光标的位置，实际上已经进行了缩放，但是只缩放光标所在的屏幕
    //例如两个1920的屏幕横连，实际宽度是3840,进行1.25缩放后实际宽度是3072、但是屏幕1上的点的范围是0～1920,屏幕二上的范围是1920～3840
//    QPoint centerPos =   this->cursor().pos(); //不清楚为啥有些时候此接口获取的值是两个屏幕都缩放了，有些时候只有一个屏幕缩放
    qreal ration = this->devicePixelRatioF();
    QPoint centerPos = m_cursorPos / ration;
//    qDebug() << "0 centerPos: " << centerPos << "m_cursorPos: " << m_cursorPos;
//    centerPos = QPoint(std::max(centerPos.x() - this->window()->x(), 0),
//                       std::max(centerPos.y() - this->window()->y(), 0));
//    qDebug() << "1 centerPos: " << centerPos;
    QPainter painter(this);
    //获取背景图片
    const QPixmap &fullscreenImgFile = TempFile::instance()->getFullscreenPixmap();
//    qDebug() << "fullscreenImgFile.size(): " << fullscreenImgFile.size();
    QImage fullscreenImg = fullscreenImgFile.toImage();
//    qDebug() << "1 fullscreenImg.size(): " << fullscreenImg.size();

    //将背景图片按屏幕缩放比进行缩放
    fullscreenImg =  fullscreenImg.scaled(static_cast<int>(fullscreenImg.width() / ration),
                                          static_cast<int>(fullscreenImg.height() / ration), Qt::KeepAspectRatio);

//    qDebug() << "2 fullscreenImg.size(): " << fullscreenImg.size();
    //获取所有屏幕的信息
//    QList<Utils::ScreenInfo> screensInfo = Utils::getScreensInfo();
//    for (int i = 0; i < screensInfo.size(); i++) {
//        //判断当前点在哪块屏幕上
//        if (centerPos.x() > screensInfo[i].x && centerPos.x() < screensInfo[i].x + screensInfo[i].width &&
//                centerPos.y() > screensInfo[i].y && centerPos.y() < screensInfo[i].y + screensInfo[i].height) {
//            centerPos.setX(static_cast<int>((centerPos.x() - screensInfo[i].x) * ration + screensInfo[i].x ));
//            centerPos.setY(static_cast<int>((centerPos.y() - screensInfo[i].y) * ration + screensInfo[i].y ));
//            break;
//        }
//    }
//    qDebug() << "2 centerPos: " << centerPos;
//    centerPos = centerPos*ration;
//    qDebug() << "3 centerPos: " << centerPos;
    //返回centerPos位置的像素颜色
    const QRgb centerRectRgb = fullscreenImg.pixel(centerPos);
    QRect tempRec = QRect(centerPos.x() - IMG_WIDTH / 2, centerPos.y() - IMG_WIDTH / 2, IMG_WIDTH, IMG_WIDTH) ;
    //qDebug() << " tempRec: " <<  tempRec;
    QPixmap zoomPix = QPixmap(fullscreenImgFile).scaled(
                          fullscreenImg.width(), fullscreenImg.height()).copy(tempRec);

    zoomPix = zoomPix.scaled(QSize(INDICATOR_WIDTH,  INDICATOR_WIDTH),
                             Qt::KeepAspectRatio);
    painter.drawPixmap(QRect(5, 5, INDICATOR_WIDTH, INDICATOR_WIDTH), zoomPix);
    painter.drawPixmap(m_centerRect, QPixmap(":/images/action/center_rect.png"));
    painter.drawPixmap(m_globalRect, QPixmap(":/images/action/magnifier.png"));
    m_lastCenterPosBrush = QBrush(QColor(qRed(centerRectRgb),
                                         qGreen(centerRectRgb), qBlue(centerRectRgb)));
    painter.fillRect(QRect(INDICATOR_WIDTH / 2 + 2, INDICATOR_WIDTH / 2 + 2,
                           CENTER_RECT_WIDTH - 4, CENTER_RECT_WIDTH - 4), m_lastCenterPosBrush);
    painter.fillRect(QRect(5, INDICATOR_WIDTH - 9, INDICATOR_WIDTH, BOTTOM_RECT_HEIGHT),
                     QBrush(QColor(0, 0, 0, 125)));
    QFont posFont;
    posFont.setPixelSize(9);
    painter.setFont(posFont);
    painter.setPen(QColor(Qt::white));
    QTextOption posTextOption;
    posTextOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    painter.drawText(QRectF(5, INDICATOR_WIDTH - 10, INDICATOR_WIDTH, INDICATOR_WIDTH),
                     QString("%1, %2").arg(m_cursorPos.x()).arg(m_cursorPos.y()), posTextOption);
}

void ZoomIndicator::showMagnifier(QPoint pos)
{
    if (Utils::isWaylandMode && !m_isOpenWM) {
        m_zoomIndicatorGL->showMagnifier(pos);
        return;
    }

    this->show();

    this->move(pos);
    //qDebug() << "this->pos(): " << this->pos();
}

void ZoomIndicator::hideMagnifier()
{
    if (Utils::isWaylandMode && !m_isOpenWM) {
        m_zoomIndicatorGL->hide();
        return;
    }
    this->hide();
}

void ZoomIndicator::setCursorPos(QPoint pos)
{
    m_cursorPos = pos;
    //qDebug() << "0 pos: " << pos << "m_cursorPos: " << m_cursorPos;
}
