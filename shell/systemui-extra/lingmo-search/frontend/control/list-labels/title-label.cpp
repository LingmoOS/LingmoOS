/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include "title-label.h"
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#define UNFOLD_LABEL_HEIGHT 30
#define NUM_LIMIT_SHOWN_DEFAULT 5
using namespace LingmoUISearch;
TitleLabel::TitleLabel(QWidget * parent) : QLabel(parent) {
    initUi();
    initConnections();
}

void TitleLabel::setShowMoreLableVisible()
{
    m_showMoreLabel->setVisible(true);
    m_showMoreLabel->setLabel();
}

void TitleLabel::initUi()
{
    m_radius = qApp->style()->property("normalRadius").toInt();
    connect(qApp, &QApplication::paletteChanged, this, [&]() {
        m_radius = qApp->style()->property("normalRadius").toInt();
    });
    this->setContentsMargins(8, 0, 0, 0);
    this->setFixedHeight(24);
    m_titleLyt = new QHBoxLayout(this);
    this->setLayout(m_titleLyt);
    m_showMoreLabel = new ShowMoreLabel(this);
    m_showMoreLabel->setFixedHeight(UNFOLD_LABEL_HEIGHT);
    m_showMoreLabel->adjustSize();
    m_showMoreLabel->hide();
    m_titleLyt->addStretch();
    m_titleLyt->addWidget(m_showMoreLabel);
}

void TitleLabel::initConnections()
{
    connect(m_showMoreLabel, &ShowMoreLabel::showMoreClicked, this, &TitleLabel::showMoreClicked);
    connect(m_showMoreLabel, &ShowMoreLabel::retractClicked, this, &TitleLabel::retractClicked);

}

void TitleLabel::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QRect rect = this->rect();
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setBrush(opt.palette.color(QPalette::Text));
    p.setOpacity(0.04);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect, m_radius, m_radius);
    return QLabel::paintEvent(event);
}

void TitleLabel::onListLengthChanged(const int &length)
{
    m_showMoreLabel->setVisible(length > NUM_LIMIT_SHOWN_DEFAULT);
}

void TitleLabel::labelReset()
{
    m_showMoreLabel->resetLabel();
}
