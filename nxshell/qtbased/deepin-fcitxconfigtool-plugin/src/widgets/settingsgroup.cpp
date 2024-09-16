// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsgroup.h"
#include "settingsitem.h"
#include "settingsheaderitem.h"
#include "imactivityitem.h"
#include "settingshead.h"
#include "utils.h"
#include "imsettingsitem.h"
#include <DBackgroundGroup>

#include <QVBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

namespace dcc_fcitx_configtool {
namespace widgets {

FcitxSettingsGroup::FcitxSettingsGroup(QFrame *parent, BackgroundStyle bgStyle)
    : FcitxTranslucentFrame(parent)
    , m_layout(new QVBoxLayout)
    , m_headerItem(nullptr)
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *vLayout = m_layout;
    if (GroupBackground == bgStyle) {
        vLayout = new QVBoxLayout;
        m_bggroup = new DBackgroundGroup(m_layout);
        m_bggroup->setBackgroundRole(QPalette::Window);
        m_bggroup->setItemSpacing(1);
        m_bggroup->setUseWidgetBackground(false);
        vLayout->addWidget(m_bggroup);
        vLayout->setContentsMargins(0, 0, 0, 0);
    }

    m_bgStyle = bgStyle;
    setLayout(vLayout);
}

FcitxSettingsGroup::FcitxSettingsGroup(const QString &title, QFrame *parent)
    : FcitxSettingsGroup(parent)
{
    setHeaderVisible(!title.isEmpty());
    setAccessibleName(title);

    m_headerItem->setTitle(title);
}

FcitxSettingsGroup::~FcitxSettingsGroup()
{
    if (m_headerItem)
        m_headerItem->deleteLater();
}

void FcitxSettingsGroup::setHeaderVisible(const bool visible)
{
    if (visible) {
        if (!m_headerItem)
            m_headerItem = new FcitxSettingsHeaderItem;
        insertItem(0, m_headerItem);
    } else {
        if (m_headerItem) {
            m_headerItem->deleteLater();
            m_headerItem = nullptr;
        }
    }
}

void FcitxSettingsGroup::insertItem(const int index, FcitxSettingsItem *item)
{
    if (ItemBackground == m_bgStyle) {
        //当SettingsItem 被加入　FcitxSettingsGroup　时，为其加入背景
        FcitxIMActivityItem *pItem = dynamic_cast<FcitxIMActivityItem*>(item);
        if(pItem == nullptr) {
            item->addBackground();
            //m_layout->addSpacing(5);
        }
    }
    m_layout->insertWidget(index, item, 1, Qt::AlignVCenter);
    item->installEventFilter(this);

    FcitxIMSettingsItem *mItem = dynamic_cast<FcitxIMSettingsItem *>(item);
    if (mItem) {
        connect(mItem, &FcitxIMSettingsItem::itemClicked, [=](FcitxIMSettingsItem *myItem) {
            int i = itemCount();
            for (int j = 0; j < i; ++j) {
                if (this->getItem(j) != myItem) {
                    FcitxIMSettingsItem *Titem = dynamic_cast<FcitxIMSettingsItem *>(this->getItem(j));
                    if (Titem) {
                        Titem->setItemSelected(false);
                    }
                }
            }
        });
    }
}

void FcitxSettingsGroup::appendItem(FcitxSettingsItem *item)
{
    insertItem(m_layout->count(), item);
}

void FcitxSettingsGroup::appendItem(FcitxSettingsItem *item, BackgroundStyle bgStyle)
{
    if ((ItemBackground == bgStyle) && (m_bgStyle == ItemBackground)) {
        //当SettingsItem 被加入　FcitxSettingsGroup　时，为其加入背景
        item->addBackground();
    }
    m_layout->addSpacing(5);
    m_layout->insertWidget(m_layout->count(), item, 8, Qt::AlignVCenter);
    item->installEventFilter(this);

    FcitxIMSettingsItem *mItem = dynamic_cast<FcitxIMSettingsItem *>(item);
    if (mItem) {
        connect(mItem, &FcitxIMSettingsItem::itemClicked, [=](FcitxIMSettingsItem *myItem) {
            int i = itemCount();
            for (int j = 0; j < i; ++j) {
                if (this->getItem(j) != myItem) {
                    FcitxIMSettingsItem *Titem = dynamic_cast<FcitxIMSettingsItem *>(this->getItem(j));
                    if (Titem) {
                        Titem->setItemSelected(false);
                    }
                }
            }
        });
    }
}

void FcitxSettingsGroup::removeItem(FcitxSettingsItem *item)
{
    if (!item)
        return;
    m_layout->removeWidget(item);

    item->removeEventFilter(this);

    for (int index = 0; index < itemCount(); index++) {
        FcitxSettingsItem* item = qobject_cast<FcitxSettingsItem *>(m_layout->itemAt(index)->widget());
        FcitxIMActivityItem *pItem = dynamic_cast<FcitxIMActivityItem*>(item);
        if(pItem != nullptr) {
            if(index == 0) {
                pItem->setIndex(FcitxIMActivityItem::firstItem);
                if(itemCount() == 1) {
                    pItem->setIndex(FcitxIMActivityItem::onlyoneItem);
                }
            } else if(index == itemCount() -1){
                pItem->setIndex(FcitxIMActivityItem::lastItem);
            } else {
                pItem->setIndex(FcitxIMActivityItem::otherItem);
            }
        }
    }
}

int FcitxSettingsGroup::indexOf(FcitxSettingsItem *item)
{
    return m_layout->indexOf(item);
}

void FcitxSettingsGroup::moveItem(FcitxSettingsItem *item, const int index)
{
    const int oldIndex = m_layout->indexOf(item);
    if (oldIndex == index)
        return;

    m_layout->removeWidget(item);
    m_layout->insertWidget(index, item);
}

void FcitxSettingsGroup::setSpacing(const int spaceing)
{
    m_layout->setSpacing(spaceing);
    if (m_bggroup)
        m_bggroup->setItemSpacing(spaceing);
}

int FcitxSettingsGroup::itemCount() const
{
    return m_layout->count();
}

void FcitxSettingsGroup::clear()
{
    const int index = m_headerItem ? 1 : 0;
    const int count = m_layout->count();

    for (int i(index); i != count; ++i) {
        QLayoutItem *item = m_layout->takeAt(index);
        QWidget *w = item->widget();
        if(w != nullptr) {
            w->removeEventFilter(this);
            w->setParent(nullptr);
            w->deleteLater();
        }
        delete item;
    }
}

void FcitxSettingsGroup::switchItem(int start, int end)
{
    FcitxSettingsItem* selectItem = getItem(start);
    FcitxIMActivityItem *mCurrentItem = dynamic_cast<FcitxIMActivityItem *>(getItem(start));
    m_layout->removeWidget(selectItem);
    m_layout->insertWidget(end, selectItem);
    for (int i = 0; i < m_layout->count(); i++) {
        FcitxIMActivityItem *mItem = dynamic_cast<FcitxIMActivityItem *>(getItem(i));
        if(nullptr == mItem) {
            return;
        }
        if (i == 0) {
            if(m_layout->count() == 1) {
                mItem->setIndex(FcitxIMActivityItem::onlyoneItem);
            } else {
                mItem->setIndex(FcitxIMActivityItem::firstItem);
            }

        } else if (i == m_layout->count() - 1) {
            mItem->setIndex(FcitxIMActivityItem::lastItem);
        } else {
            mItem->setIndex(FcitxIMActivityItem::otherItem);
        }
        mItem->setDraged(false);
        mItem->update(mItem->rect());
    }

    emit switchPosition(mCurrentItem->m_item, end);
}

void FcitxSettingsGroup::setVerticalPolicy()
{
    QSizePolicy policy;
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    setSizePolicy(policy);
}

void FcitxSettingsGroup::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_switchAble || event->pos().y() < 0 || event->pos().y() > rect().height()) {
        return QWidget::mouseMoveEvent(event);
    }
//    if((QDateTime::currentDateTime().toTime_t() - m_time.toTime_t() < 1) || !m_isPressed) {
//        m_isPressed = false;
//        return QWidget::mouseMoveEvent(event);
//    }
    if(m_isPressed) {
        FcitxSettingsItem* selectItem = getItem(m_selectIndex);
        selectItem->move(selectItem->mapTo(this, QPoint(selectItem->x(), selectItem->rect().topRight().y() + (event->pos().y() - m_lastYPosition))));
        for(int index =0; index < itemCount(); index++) {
            FcitxSettingsItem* item = getItem(index);
            QRect itemRect = item->rect();
            QRect selectRect = selectItem->rect();
            QPoint itemTopLeftToThis = item->mapTo(this, itemRect.topLeft());
            QPoint selecBottomLeft = selectItem->mapTo(this, selectRect.bottomLeft() + QPoint(10,2));
            QPoint selecTopLeft = selectItem->mapTo(this, selectRect.topLeft() + QPoint(10,2));
            QRect r1 = QRect(itemRect.x() + itemTopLeftToThis.x(), itemRect.y() + itemTopLeftToThis.y(), itemRect.width(), itemRect.height());
            if((r1.contains(selecBottomLeft) || r1.contains(selecTopLeft)) && index != m_selectIndex) {
                if(m_lastItem != item && m_lastItem != nullptr) {
                    m_lastItem->setDraged(false);
                    m_lastItem->update(m_lastItem->rect());
                }
                item->move(item->mapTo(this, QPoint(item->x(), item->rect().topRight().y() - (event->pos().y() - m_lastYPosition))));
                item->setDraged(true);
                m_lastItem = item;
            }
        }
    }
    m_lastYPosition = event->pos().y();
    //return QWidget::mouseMoveEvent(event);
}

void FcitxSettingsGroup::mousePressEvent(QMouseEvent *event)
{
    if(!m_switchAble) {
        return QWidget::mousePressEvent(event);
    }
    m_isPressed = true;
    for(int index =0; index < itemCount(); index++) {
        FcitxSettingsItem* item = getItem(index);
        QRect r = item->rect();
        QPoint p = item->mapTo(this, r.topLeft());
        QRect r1 = QRect(r.x() + p.x(), r.y() + p.y(),r.width(), r.height());
        if(r1.contains(event->pos())) {
            qDebug() << "index = " << index;
            item->setDraged(true);
            m_selectIndex = index;
        }
    }
    m_lastYPosition = event->pos().y();
    m_time = QDateTime::currentDateTime();
    return QWidget::mousePressEvent(event);
}

void FcitxSettingsGroup::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_switchAble) {
        return QWidget::mouseReleaseEvent(event);
    }
    m_isPressed = false;
    FcitxSettingsItem* selectItem = getItem(m_selectIndex);
    selectItem->setDraged(false);

    QRect selectRect = selectItem->rect();
    QPoint selecTopLeft = selectItem->mapTo(this, selectRect.topLeft() + QPoint(10,0));
    int count = selecTopLeft.y() / selectItem->height();
    if(count < 0) {
        count = 0;
    }
    if(selecTopLeft.y() % selectItem->height() > (selectItem->height() / 2)) {
        count ++;
    }
    m_lastItem = nullptr;
    switchItem(m_selectIndex,count);
    return QWidget::mouseReleaseEvent(event);
}


FcitxSettingsItem *FcitxSettingsGroup::getItem(int index)
{
    if (index < 0)
        return nullptr;

    if (index < itemCount()) {
        return qobject_cast<FcitxSettingsItem *>(m_layout->itemAt(index)->widget());
    }

    return nullptr;
}

void FcitxSettingsGroup::insertWidget(QWidget *widget)
{
    m_layout->insertWidget(m_layout->count(), widget);
}
} // namespace widgets
} // namespace dcc_fcitx_configtool
