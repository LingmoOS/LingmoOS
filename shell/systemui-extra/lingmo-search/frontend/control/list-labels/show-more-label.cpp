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
#include "show-more-label.h"
#include "icon-loader.h"
#include <QEvent>
#include <QDebug>
#include <QIcon>
using namespace LingmoUISearch;
ShowMoreLabel::ShowMoreLabel(QWidget *parent) : QWidget(parent) {
    initUi();
    m_timer = new QTimer;
}

void ShowMoreLabel::resetLabel() {
    m_isOpen = false;
    m_iconLabel->setPixmap(IconLoader::loadIconQt("lingmo-down-symbolic", QIcon(":/res/icons/lingmo-down-symbolic.svg")).pixmap(QSize(16, 16)));
}

void ShowMoreLabel::setLabel()
{
    m_isOpen = true;
    m_iconLabel->setPixmap(IconLoader::loadIconQt("lingmo-up-symbolic", QIcon(":/res/icons/lingmo-up-symbolic.svg")).pixmap(QSize(16, 16)));
}

/**
 * @brief ShowMoreLabel::getExpanded 获取当前是否是展开状态
 * @return true已展开，false已收起
 */
bool ShowMoreLabel::getExpanded() {
    return m_isOpen;
}

void ShowMoreLabel::initUi() {
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, QColor(55, 144, 250, 255));
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 6);
    m_iconLabel = new QLabel(this);
    m_iconLabel->setPixmap(IconLoader::loadIconQt("lingmo-down-symbolic", QIcon(":/res/icons/lingmo-down-symbolic.svg")).pixmap(QSize(16, 16)));
    m_iconLabel->setCursor(QCursor(Qt::PointingHandCursor));
    m_iconLabel->installEventFilter(this);
//    m_loadingIconLabel = new QLabel(this); //使用图片显示加载状态时，取消此label的注释
//    m_loadingIconLabel->setFixedSize(18, 18);
//    m_loadingIconLabel->hide();
    m_layout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_layout->addWidget(m_iconLabel);
    m_iconLabel->setPalette(pal);
    m_iconLabel->setCursor(QCursor(Qt::PointingHandCursor));
    m_iconLabel->setProperty("useIconHighlightEffect", 0x02);
    m_iconLabel->setProperty("iconHighlightEffectMode", 1);
//    m_layout->addWidget(m_loadingIconLabel);
}

bool ShowMoreLabel::eventFilter(QObject *watched, QEvent *event) {
    if(watched == m_iconLabel) {
        if(event->type() == QEvent::MouseButtonPress) {
            if(! m_timer->isActive()) {
                if(!m_isOpen) {
                    m_iconLabel->setPixmap(IconLoader::loadIconQt("lingmo-up-symbolic", QIcon(":/res/icons/lingmo-up-symbolic.svg")).pixmap(QSize(16, 16)));
                    m_isOpen = true;
                    Q_EMIT this->showMoreClicked();
                } else {
                    m_iconLabel->setPixmap(IconLoader::loadIconQt("lingmo-down-symbolic", QIcon(":/res/icons/lingmo-down-symbolic.svg")).pixmap(QSize(16, 16)));
                    m_isOpen = false;
                    Q_EMIT this->retractClicked();
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
