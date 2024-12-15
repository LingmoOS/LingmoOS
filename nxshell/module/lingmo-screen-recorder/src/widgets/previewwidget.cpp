// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwidget.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

PreviewWidget::PreviewWidget(const QRect &rect, QWidget *parent) : QWidget(parent), m_previewRect(rect)
{
    m_recordHeight = rect.height();
    m_recordWidth = rect.width();
    m_recordX = rect.x();
    m_recordY = rect.y();
    QRect mainRect = QApplication::desktop()->screen()->geometry();
    m_maxHeight = mainRect.height() * 7 / 10 ; //最大预览高度
    m_maxWidth = mainRect.width() / 4 ; //最大预览宽度
}
//初始化位置，大小
void PreviewWidget::initPreviewWidget()
{
    m_previewRect = previewGeomtroy();
    setGeometry(m_previewRect);
}

//更新捕捉区域的大小及位置
void PreviewWidget::updatePreviewSize(const QRect &rect)
{
    m_recordHeight = rect.height();
    m_recordWidth = rect.width();
    m_recordX = rect.x();
    m_recordY = rect.y();
    initPreviewWidget();
}

PreviewWidget::PostionStatus PreviewWidget::getPreviewPostion()
{
    return m_StatusPos;
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    //paint
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawImage(rect(), m_currentPix);
}
//设置位置状态
void PreviewWidget::setPreviewWidgetStatusPos(PostionStatus statusPos)
{
    m_StatusPos = statusPos;
}
//更新图片
void PreviewWidget::updateImage(const QImage &image)
{
    int previewHeight = 0; //预览高度
    int previewWidth = 0; //预览宽度
    int imageHight = int(image.height() / m_screenRatio);
    int imageWidth = int(image.width() / m_screenRatio);
    bool unchanged = false; //是否高宽不变
    //计算图片缩放后的预览宽高
    if (imageHight <= m_maxHeight && imageWidth <= m_maxWidth) {
        previewHeight = imageHight;
        previewWidth = imageWidth;
        unchanged = true;
    } else if (imageHight <= m_maxHeight && imageWidth > m_maxWidth) {
        previewHeight = m_maxWidth * imageHight / imageWidth;
        previewWidth = m_maxWidth;
    } else if (imageHight > m_maxHeight && imageWidth <= m_maxWidth) {
        previewHeight = m_maxHeight;
        previewWidth = m_maxHeight * imageWidth / imageHight;
    } else if (imageHight > m_maxHeight && imageWidth > m_maxWidth) {
        previewHeight = m_maxWidth * imageHight / imageWidth; //假设宽度为最大
        previewWidth = m_maxHeight * imageWidth / imageHight;//假设高度为最大
        if (previewHeight <= m_maxHeight) {
            previewWidth = m_maxWidth;
        } else if (previewWidth <= m_maxWidth) {
            previewHeight = m_maxHeight;
        }
    }
    int hightDiff_t = previewHeight - m_previewRect.height(); //高度差
    //判断向上是否超出屏幕外
    int previewY = m_previewRect.y() - hightDiff_t;
    if (previewY <= 0) {
        previewY = 0;
        if (m_StatusPos == INSIDE)
            previewHeight = m_recordY + m_recordHeight - 20; //设置此时的预览高度=捕捉区域的高度+y轴坐标
        else
            previewHeight = m_recordY + m_recordHeight;//设置此时的预览高度=捕捉区域的高度+y轴坐标
    }
    m_previewRect.setY(previewY); //重新设置y坐标
    m_previewRect.setHeight(previewHeight);//重新设置预览高度
    int widthDiff_t = m_previewRect.width() - previewWidth; //宽度差
    if (m_StatusPos == LEFT) {//预览框在左
        m_previewRect.setX(m_previewRect.x() + widthDiff_t); //重新设置x坐标
    } else if (m_StatusPos == RIGHT) {//预览框在右
        m_previewRect.setX(m_previewRect.x());
    } else {//预览框在内部
        m_previewRect.setX(m_previewRect.x() + widthDiff_t);
    }
    m_previewRect.setWidth(previewWidth);//重新设置预览宽度
    setGeometry(m_previewRect);
    if (true == unchanged) {//高宽不变,不进行宽高等比缩放
        m_currentPix = image;
    } else {
        QImage tempImage = image.scaled(previewWidth, previewHeight, Qt::
                                        KeepAspectRatioByExpanding, Qt::SmoothTransformation);//以预览框的宽高等比例缩放
        m_currentPix = image;
    }
    update();
}

//根据捕捉区域大小初始化预览框位置大小
QRect PreviewWidget::previewGeomtroy()
{
    QRect rt;
    int previewHeight = 0; //预览高度
    int previewWidth = 0; //预览宽度
    if (m_recordHeight <= m_maxHeight && m_recordWidth <= m_maxWidth) {
        previewHeight = m_recordHeight;
        previewWidth = m_recordWidth;
        rt = calculatePreviewPosition(previewWidth, previewHeight);
    } else if (m_recordHeight <= m_maxHeight && m_recordWidth > m_maxWidth) {
        previewHeight = m_maxWidth * m_recordHeight / m_recordWidth;;
        previewWidth = m_maxWidth;
        rt = calculatePreviewPosition(previewWidth, previewHeight);
    } else if (m_recordHeight > m_maxHeight && m_recordWidth <= m_maxWidth) {
        previewHeight = m_maxHeight;
        previewWidth = m_maxHeight * m_recordWidth / m_recordHeight;
        rt = calculatePreviewPosition(previewWidth, previewWidth);
    } else if (m_recordHeight > m_maxHeight && m_recordWidth > m_maxWidth) {
        previewHeight = m_maxWidth * m_recordHeight / m_recordWidth; //假设宽度为最大
        previewWidth = m_maxHeight * m_recordWidth / m_recordHeight;//假设高度为最大
        if (previewHeight <= m_maxHeight) {
            previewWidth = m_maxWidth;
            rt = calculatePreviewPosition(previewWidth, previewHeight);
        } else if (previewWidth <= m_maxWidth) {
            previewHeight = m_maxHeight;
            rt = calculatePreviewPosition(previewWidth, previewHeight);
        }
    }
    return rt;
}
//判定在左还是在右
QRect PreviewWidget::calculatePreviewPosition(int previewWidth, int previewHeight)
{
    QRect rt;
    int previewY = m_recordY + m_recordHeight - previewHeight;
    int rightX = m_recordX + m_recordWidth + 20; //根据捕捉区域计算预览向右的起始x
    int leftX = m_recordX - previewWidth - 20; //根据捕捉区域计算预览向左的起始x
    int rightMargin = m_screenWidth - rightX; //右边距
    int leftMargin = m_recordX; //左边距
    //判断位置是左还是右
    if (rightMargin >= leftMargin && rightMargin >= previewWidth + 1) {
        rt.setX(rightX);
        rt.setY(previewY);
        rt.setWidth(previewWidth);
        rt.setHeight(previewHeight);
        setPreviewWidgetStatusPos(RIGHT);//设置位置状态
    } else if (rightMargin < leftMargin && leftMargin >= previewWidth + 1) {
        if (rightMargin >= previewWidth + 1) { //默认优先显示在右边
            rt.setX(rightX);
            rt.setY(previewY);
            rt.setWidth(previewWidth);
            rt.setHeight(previewHeight);
            setPreviewWidgetStatusPos(RIGHT);
        } else {
            rt.setX(leftX);
            rt.setY(previewY);
            rt.setWidth(previewWidth);
            rt.setHeight(previewHeight);
            setPreviewWidgetStatusPos(LEFT);
        }
    } else {
        int nX = m_recordX + m_recordWidth - previewWidth - 20; //根据捕捉区域计算内部预览的起始x
        int ny = m_recordY + m_recordHeight - previewHeight - 20;//根据捕捉区域计算内部预览的起始y
        rt.setX(nX);
        rt.setY(ny);
        rt.setWidth(previewWidth);
        rt.setHeight(previewHeight);
        setPreviewWidgetStatusPos(INSIDE);
    }
    return rt;
}
//设置屏幕宽度
void PreviewWidget::setScreenInfo(int screenWidth, double screenRatio)
{
    m_screenWidth = screenWidth;
    m_screenRatio = screenRatio;
}
