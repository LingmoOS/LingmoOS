/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "chameleonbutton.h"
#include "chameleon.h"
#include "kwinutils.h"
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>
#include <QTimer>
#include <QHoverEvent>
#include <QPainter>
#include <QDebug>
#include <QX11Info>

#include "workspace.h"

#define LONG_PRESS_TIME     300
#define OUT_RELEASE_EVENT   100

ChameleonButton::ChameleonButton(KDecoration2::DecorationButtonType type, const QPointer<KDecoration2::Decoration> &decoration, QObject *parent)
    : KDecoration2::DecorationButton(type, decoration, parent)
{
    auto c = decoration->client().data();
    qDebug()<<__FUNCTION__<<__LINE__<<"windowId: "<<c->windowId();

    m_type = type;

    switch (type) {
    case KDecoration2::DecorationButtonType::Menu:
        break;
    case KDecoration2::DecorationButtonType::Minimize:
        setVisible(c->isMinimizeable());
        connect(c, &KDecoration2::DecoratedClient::minimizeableChanged, this, &ChameleonButton::setVisible);
        break;
    case KDecoration2::DecorationButtonType::Maximize:
        setVisible(c->isMaximizeable());
        connect(c, &KDecoration2::DecoratedClient::maximizeableChanged, this, &ChameleonButton::setVisible);
        break;
    case KDecoration2::DecorationButtonType::Close:
        setVisible(c->isCloseable());
        connect(c, &KDecoration2::DecoratedClient::closeableChanged, this, &ChameleonButton::setVisible);
        break;
    default: // 隐藏不支持的按钮
        setVisible(false);
        break;
    }
    if (m_type == KDecoration2::DecorationButtonType::Maximize) {
        connect(KWinUtils::compositor(), SIGNAL(compositingToggled(bool)), this, SLOT(onCompositorChanged(bool)));
    }
}

ChameleonButton::~ChameleonButton()
{
    KWinUtils::setSplitMenuKeepShowing(false);
    KWinUtils::hideSplitMenu(false);
}

KDecoration2::DecorationButton *ChameleonButton::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    return new ChameleonButton(type, decoration, parent);
}

void ChameleonButton::paint(QPainter *painter, const QRect &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    Chameleon *decoration = qobject_cast<Chameleon*>(this->decoration());

    if (!decoration)
        return;

    const QRect &rect = geometry().toRect();

    painter->save();

    auto c = decoration->client().data();

    QIcon::Mode state = QIcon::Normal;

    if (!isEnabled()) {
        state = QIcon::Disabled;
    } else if (isPressed()) {
        state = QIcon::Selected;
    } else if (isHovered()) {
        state = QIcon::Active;
    }

    switch (type()) {
    case KDecoration2::DecorationButtonType::Menu: {
        c->icon().paint(painter, rect);
        break;
    }
    case KDecoration2::DecorationButtonType::ApplicationMenu: {
        decoration->menuIcon().paint(painter, rect, Qt::AlignCenter, state);
        break;
    }
    case KDecoration2::DecorationButtonType::Minimize: {
        decoration->minimizeIcon().paint(painter, rect, Qt::AlignCenter, state);
        break;
    }
    case KDecoration2::DecorationButtonType::Maximize: {
        if (isChecked())
            decoration->unmaximizeIcon().paint(painter, rect, Qt::AlignCenter, state);
        else
            decoration->maximizeIcon().paint(painter, rect, Qt::AlignCenter, state);
        break;
    }
    case KDecoration2::DecorationButtonType::Close: {
        decoration->closeIcon().paint(painter, rect, Qt::AlignCenter, state);
        break;
    }
    default:
        break;
    }

    painter->restore();
}

void ChameleonButton::hoverEnterEvent(QHoverEvent *event)
{
    if (!m_isMaxAvailble)
        return;

    if (KWinUtils::instance()->isCompositing()) {
        Chameleon *decoration = qobject_cast<Chameleon*>(this->decoration());
        if (decoration) {
            effect = decoration->effect();
            if (effect && !effect->isUserMove()) {
                KDecoration2::DecorationButton::hoverEnterEvent(event);

                if (!contains(event->posF())) {
                    return;
                }
                if (m_type == KDecoration2::DecorationButtonType::Maximize) {
                    if (KWinUtils::instance()->isCompositing()) {
                        auto c = decoration->client().data();
                        if (c) {
                            uint32_t wid = effect->isWaylandClient() ? c->decorationId() : c->windowId();
                            QRect button_rect(QPoint(geometry().x() + effect->pos().x(), effect->pos().y()),
                                              QSize(geometry().width(), decoration->titleBarHeight()));
                            KWinUtils::showSplitMenu(button_rect, wid);
                        }
                    }
                    decoration->requestHideToolTip();
                }
            }
        }
    } else {
        KDecoration2::DecorationButton::hoverEnterEvent(event);
    }
}

void ChameleonButton::hoverLeaveEvent(QHoverEvent *event)
{
    if (KWinUtils::instance()->isCompositing()) {
        Chameleon *decoration = qobject_cast<Chameleon*>(this->decoration());
        if (decoration) {
            effect = decoration->effect();
            if (max_hover_timer && m_type == KDecoration2::DecorationButtonType::Maximize) {
                max_hover_timer->stop();
            }
            if (effect && !effect->isUserMove()) {
                KDecoration2::DecorationButton::hoverLeaveEvent(event);
                if (m_type == KDecoration2::DecorationButtonType::Maximize) {
                    KWinUtils::hideSplitMenu(true);
                }
            }
        }
    } else {
        KDecoration2::DecorationButton::hoverLeaveEvent(event);
    }
}

void ChameleonButton::mousePressEvent(QMouseEvent *event)
{
    KDecoration2::DecorationButton::mousePressEvent(event);
    if (m_type == KDecoration2::DecorationButtonType::Maximize) {
        if (!max_timer) {
            max_timer = new QTimer();
            max_timer->setSingleShot(true);
            connect(max_timer, &QTimer::timeout, [this] {
                if (m_isMaxAvailble) {
                    m_isMaxAvailble = false;
                    Chameleon *decoration = qobject_cast<Chameleon*>(this->decoration());
                    if (decoration) {
                        effect = decoration->effect();
                        if (effect) {
                            auto c = decoration->client().data();
                            if (c) {
                                uint32_t wid = effect->isWaylandClient() ? c->decorationId() : c->windowId();
                                KWinUtils::setSplitMenuKeepShowing(true);
                                QRect button_rect(QPoint(geometry().x() + effect->pos().x(), effect->pos().y()),
                                                  QSize(geometry().width(), decoration->titleBarHeight()));
                                KWinUtils::showSplitMenu(button_rect, wid);
                            }
                        }
                    }
                }
            });
            max_timer->start(LONG_PRESS_TIME);
            m_mousePosX = event->pos().x();
        } else {
            max_timer->start(LONG_PRESS_TIME);
            m_mousePosX = event->pos().x();
        }
    }
}

void ChameleonButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_type == KDecoration2::DecorationButtonType::Maximize) {
        if (max_timer) {
            max_timer->stop();
        }
        if (!geometry().contains(event->localPos()))
            KWinUtils::setSplitMenuKeepShowing(false);
        if (!m_isMaxAvailble) {
            event->setLocalPos(QPointF(event->localPos().x() - OUT_RELEASE_EVENT, event->localPos().y()));
        }
        KWinUtils::hideSplitMenu(false);
        KWinUtils::setSplitMenuKeepShowing(false);
    }
    KDecoration2::DecorationButton::mouseReleaseEvent(event);
    m_isMaxAvailble = true;
}

void ChameleonButton::onCompositorChanged(bool active)
{
    if (!active) {
        KWinUtils::hideSplitMenu(false);
    }
}
