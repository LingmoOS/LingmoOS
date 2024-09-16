// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "icondframe.h"
#include <QPainter>
#include <QDate>
#include <QSvgRenderer>
#include <QVBoxLayout>

#include <DApplicationHelper>

#include "../globaldef.h"
#include "../interface/reply.h"

IconDFrame::IconDFrame(QWidget *parent)
    : DFrame(parent)
    , m_Dayrenderer(new QSvgRenderer())
    , m_Weekrenderer(new QSvgRenderer())
    , m_Monthrenderer(new QSvgRenderer())
    , m_backgroundrenderer(new QSvgRenderer())
{
    //设置接受tab切换
    setFocusPolicy(Qt::TabFocus);
}

IconDFrame::~IconDFrame()
{
    delete m_Dayrenderer;
    m_Dayrenderer = nullptr;

    delete m_Weekrenderer;
    m_Weekrenderer = nullptr;

    delete m_Monthrenderer;
    m_Monthrenderer = nullptr;

    delete m_backgroundrenderer;
    m_backgroundrenderer = nullptr;
}

void IconDFrame::setCenterLayout(QLayout *layout)
{
    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(25);
    vlayout->addLayout(layout);
    this->setLayout(vlayout);
}

void IconDFrame::paintEvent(QPaintEvent *event)
{
    setTheMe(DGuiApplicationHelper::instance()->themeType());
    DFrame::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    //绘制tab切换focus效果
    if (m_tabFocusIn) {
        QPen pen;
        //设置边框为2
        pen.setWidth(2);
        //设置边框颜色为活动色
        pen.setColor(DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        painter.setPen(pen);
        //设置边框矩形，由于插件widget会被助手覆盖一个像素，所以需要减去一个像素的外边
        QRect _showRect(1, 1, this->width() - 2, this->height() - 2);
        painter.drawRoundedRect(_showRect, 8, 8);
    }
    //绘制图标
    paintPixmap(painter);
    //绘制标题
    paintTitle(painter);
    painter.end();
}

void IconDFrame::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    if (isEnabled()) {
        //如果窗口为启用状态则触发窗口隐藏信号
        emit widgetIsHide();
    }
}

void IconDFrame::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    //设置focus标志为true
    //TODO 根据需求先取消tab选中效果
    m_tabFocusIn = false;
    //刷新界面
    update();
}

void IconDFrame::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    //设置focus标志为false
    m_tabFocusIn = false;
    //刷新界面
    update();
}

void IconDFrame::setTheMe(const int type)
{
    QColor titleColor;
    if (type == 0 || type == 1) {
        titleColor.setRgb(0, 0, 0);
        titleColor.setAlphaF(0.4);
    } else {
        titleColor.setRgb(255, 255, 255);
        titleColor.setAlphaF(0.4);
    }
    setTitleColor(titleColor);
}

void IconDFrame::paintPixmap(QPainter &painter)
{
    QString dayfile = QString(":/resources/DynamicIcon/day%1.svg").arg(QDate::currentDate().day());

    m_Dayrenderer->load(dayfile);

    QString weekfile = QString(":/resources/DynamicIcon/week%1.svg").arg(QDate::currentDate().dayOfWeek());
    m_Weekrenderer->load(weekfile);

    QString monthfile = QString(":/resources/DynamicIcon/month%1.svg").arg(QDate::currentDate().month());
    m_Monthrenderer->load(monthfile);

    QString backgroundfile(":/resources/DynamicIcon/calendar_bg.svg");
    m_backgroundrenderer->load(backgroundfile);

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    //draw background
    QRectF backgroundRect = QRectF(11,
                                   9,
                                   16,
                                   16);
    m_backgroundrenderer->render(&painter, backgroundRect);

    //draw month
    qreal month_x = 13;
    qreal month_y = 10;
    qreal month_w = 11;
    qreal month_h = 4;
    m_Monthrenderer->render(&painter, QRectF(month_x, month_y, month_w, month_h));

    //draw day
    qreal day_x = 14;
    qreal day_y = 14;
    qreal day_w = 9;
    qreal day_h = 7;
    m_Dayrenderer->render(&painter, QRectF(day_x, day_y, day_w, day_h));
    painter.restore();
}

void IconDFrame::paintTitle(QPainter &painter)
{
    QRect rect(34, 7, 24, 17);
    painter.save();
    painter.setPen(TitleColor());
    painter.setFont(TitleFont());
    painter.drawText(rect, Qt::AlignCenter, tr(PLUGIN_TITLE_NAME));
    painter.restore();
}

void IconDFrame::slotReceivce(QVariant data, Reply *reply)
{
    Q_UNUSED(data);
    Q_UNUSED(reply);
}

QColor IconDFrame::TitleColor() const
{
    return m_TitleColor;
}

void IconDFrame::setTitleColor(const QColor &TitleColor)
{
    m_TitleColor = TitleColor;
}

QFont IconDFrame::TitleFont()
{
    m_TitleFont.setPixelSize(12);
    return m_TitleFont;
}
