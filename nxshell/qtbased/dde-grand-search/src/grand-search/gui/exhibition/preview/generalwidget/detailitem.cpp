// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailitem.h"
#include "replicablelabel.h"

#include <DLineEdit>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QVariant>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

DetailItem::DetailItem(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(360, 36);

    m_backgroundColor = QColor(0, 0, 0, int(255*0.05));

    setContentsMargins(20, 0, 10, 0);

    m_radius = 8;

    m_tagLabel = new DLabel(this);
    m_tagLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFont tagFont = m_tagLabel->font();
    tagFont = DFontSizeManager::instance()->get(DFontSizeManager::T8, tagFont);
    m_tagLabel->setFont(tagFont);

    QColor textColor = getTagColor();
    QPalette pa = m_tagLabel->palette();
    pa.setColor(QPalette::WindowText, textColor);
    m_tagLabel->setPalette(pa);

    m_contentLabel = new ReplicableLabel(this);
    m_contentLabel->setAlignment(Qt::AlignLeft);

    QFont contectFont = m_contentLabel->font();
    contectFont = DFontSizeManager::instance()->get(DFontSizeManager::T8, contectFont);
    m_contentLabel->setFont(contectFont);

    textColor = getContentColor();
    pa = m_contentLabel->palette();
    pa.setColor(QPalette::Text, textColor);
    m_contentLabel->setPalette(pa);

    m_mainLayout = new QHBoxLayout(this);

    m_mainLayout->setSpacing(10);
    m_mainLayout->addWidget(m_tagLabel);
    m_mainLayout->addWidget(m_contentLabel);
}

DetailItem::~DetailItem()
{

}

void DetailItem::setDetailInfo(const DetailInfo &info)
{

    auto keyInfos = info.first;
    auto valueInfos = info.second;

    for (auto key : keyInfos.keys()) {
        if (DetailInfoProperty::Text == key) {
            auto text = keyInfos.value(key).toString();
            m_tagLabel->setText(text);
            m_tagLabel->adjustSize();       // 根据标签内容，调整合适大小
        } else if (DetailInfoProperty::ElideMode == key) {
            auto elideMode = keyInfos.value(key).value<Qt::TextElideMode>();
            m_tagLabel->setElideMode(elideMode);
        }
    }

    for (auto key : valueInfos.keys()) {
        if (DetailInfoProperty::Text == key) {
            auto text = valueInfos.value(key).toString();
            m_contentLabel->setText(text);
            m_contentLabel->setToolTip(text);
        } else if (DetailInfoProperty::ElideMode == key) {
            auto elideMode = valueInfos.value(key).value<Qt::TextElideMode>();
            m_contentLabel->setElideMode(elideMode);
        }
    }

    update();
}

void DetailItem::setTopRound(bool round)
{
    m_topRound = round;
}

bool DetailItem::topRound() const
{
    return m_topRound;
}

void DetailItem::setBottomRound(bool round)
{
    m_bottomRound = round;
}

bool DetailItem::bottomRound() const
{
    return m_bottomRound;
}

void DetailItem::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

QColor DetailItem::backgroundColor() const
{
    return m_backgroundColor;
}

void DetailItem::setRadius(int radius)
{
    m_radius = radius;
}

int DetailItem::radius() const
{
    return m_radius;
}

void DetailItem::setTagWidth(int width)
{
    m_tagLabel->setFixedWidth(width);
    update();
}

int DetailItem::tagWidth() const
{
    return m_tagLabel->width();
}

void DetailItem::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(m_backgroundColor);

    QRect roundRect(0, 0, m_radius * 2, m_radius * 2);
    QPainterPath path;
    int w = width();
    int h = height();

    if (m_topRound) {
        // 左上圆角
        path.moveTo(0, m_radius);
        roundRect.moveTo(0, 0);
        path.arcTo(roundRect, 180, -90);

        // 右上圆角
        path.lineTo(w - m_radius, 0);
        roundRect.moveTo(w - roundRect.width(), 0);
        path.arcTo(roundRect, 90, -90);
    } else {
        path.moveTo(0, 0);
        path.lineTo(w, 0);
    }

    if (m_bottomRound) {
        // 右下圆角
        path.lineTo(w, h - m_radius);
        roundRect.moveTo(w - roundRect.width(), h - roundRect.width());
        path.arcTo(roundRect, 0, -90);

        // 左下圆角
        path.lineTo(m_radius, h);
        roundRect.moveTo(0, h - roundRect.height());
        path.arcTo(roundRect, 270, -90);
    } else {
        path.lineTo(w, h);
        path.lineTo(0, h);
    }

    path.closeSubpath();
    p.drawPath(path);

    return QWidget::paintEvent(event);
}

QColor DetailItem::getTagColor()
{
    static  QColor tagTextColor;
    if (!tagTextColor.isValid()) {
        tagTextColor = QColor(0, 0, 0, int(255*0.7));
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            tagTextColor = QColor(255, 255, 255, int(255*0.7));
    }
    return tagTextColor;
}

QColor DetailItem::getContentColor()
{
    static  QColor contentTextColor;
    if (!contentTextColor.isValid()) {
        contentTextColor = QColor(0, 0, 0, int(255*0.9));
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            contentTextColor = QColor(255, 255, 255, int(255*0.9));
    }
    return contentTextColor;
}
