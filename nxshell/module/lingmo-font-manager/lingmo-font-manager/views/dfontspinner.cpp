// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontspinner.h"

#include <QTimer>
#include <QPainter>
#include <QtMath>
#include <QEvent>
#include <QPainterPath>

class DFontSpinnerPrivate : public QObject
{
public:
    explicit DFontSpinnerPrivate(DFontSpinner *parent);
    virtual ~DFontSpinnerPrivate();

    QList<QColor> createDefaultIndicatorColorList(QColor color);

    QTimer refreshTimer;

    double indicatorShadowOffset = 10;
    double currentDegree = 0.0;

    QList<QList<QColor>> indicatorColors;
    DFontSpinner *const q_ptr;
    Q_DECLARE_PUBLIC(DFontSpinner)
};

DFontSpinnerPrivate::DFontSpinnerPrivate(DFontSpinner *parent)
    : q_ptr(parent)
{

}

DFontSpinnerPrivate::~DFontSpinnerPrivate()
{
    refreshTimer.stop();
}

DFontSpinner::DFontSpinner(QWidget *parent)
    : QWidget(parent)
    , m_ptr(new DFontSpinnerPrivate(this))
{
    m_ptr->refreshTimer.setInterval(30);

    connect(&m_ptr->refreshTimer, &QTimer::timeout,
    this, [ = ]() {
        m_ptr->currentDegree += 14;
        update();
    });
}

DFontSpinner::~DFontSpinner()
{
    delete m_ptr;
}

/*************************************************************************
 <Function>      start
 <Description>   旋转开始
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontSpinner::start()
{
    m_ptr->currentDegree += 14;
    update();
    m_ptr->refreshTimer.start();
}

/*************************************************************************
 <Function>      stop
 <Description>   旋转暂停
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontSpinner::stop()
{
    m_ptr->refreshTimer.stop();
}

///*************************************************************************
// <Function>      setBackgroundColor
// <Description>   设置背景色
// <Author>        null
// <Input>
//    <param1>     color            Description:背景色
// <Return>        null            Description:null
// <Note>          null
//*************************************************************************/
//void DFontSpinner::setBackgroundColor(QColor color)
//{
//    setAutoFillBackground(true);
//    QPalette pal = palette();
//    pal.setColor(QPalette::Background, color);
//    setPalette(pal);
//}

/*************************************************************************
 <Function>      paintEvent
 <Description>   自绘函数
 <Author>        null
 <Input>
    <param1>     QPaintEvent*    Description:绘画事件
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontSpinner::paintEvent(QPaintEvent *)
{
    if (m_ptr->indicatorColors.isEmpty()) {
        for (int i = 0; i < 3; ++i)
            m_ptr->indicatorColors << m_ptr->createDefaultIndicatorColorList(palette().highlight().color());
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    auto degreeCurrent = m_ptr->currentDegree * 1.0;

    auto center = QRectF(rect()).center();
    auto radius = qMin(rect().width(), rect().height()) / 2.0;
    auto indicatorRadius = radius / 2 / 2 * 1.1;
    auto indicatorDegreeDelta = 360 / m_ptr->indicatorColors.count();

    for (int i = 0; i <  m_ptr->indicatorColors.count(); ++i) {
        auto colors = m_ptr->indicatorColors.value(i);
        for (int j = 0; j < colors.count(); ++j) {
            degreeCurrent = m_ptr->currentDegree - j * m_ptr->indicatorShadowOffset + indicatorDegreeDelta * i;
            auto x = (radius - indicatorRadius) * qCos(qDegreesToRadians(degreeCurrent));
            auto y = (radius - indicatorRadius) * qSin(qDegreesToRadians(degreeCurrent));

            x = center.x() + x;
            y = center.y() + y;
            auto tl = QPointF(x - 1 * indicatorRadius, y - 1 * indicatorRadius);
            QRectF rf(tl.x(), tl.y(), indicatorRadius * 2, indicatorRadius * 2);

            QPainterPath path;
            path.addEllipse(rf);

            painter.fillPath(path, colors.value(j));
        }
    }
}

void DFontSpinner::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange)
        m_ptr->indicatorColors.clear();

    QWidget::changeEvent(e);
}


/*************************************************************************
 <Function>      createDefaultIndicatorColorList
 <Description>   初始化自绘spinner控件颜色的透明度
 <Author>        null
 <Input>
    <param1>     color            Description:控件颜色
 <Return>        QList<QColor>    Description:不同透明度的color颜色组成的list
 <Note>          null
*************************************************************************/
QList<QColor> DFontSpinnerPrivate::createDefaultIndicatorColorList(QColor color)
{
    QList<QColor> colors;
    QList<int> opacitys;
    opacitys << 100 << 30 << 15 << 10 << 5 << 4 << 3 << 2 << 1;
    for (int i = 0; i < opacitys.count(); ++i) {
        color.setAlpha(255 * opacitys.value(i) / 100);
        colors << color;
    }
    return colors;
}
