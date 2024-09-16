// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifywidget.h"
#include "notification/persistence.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QLabel>

#include "itemdelegate.h"
#include "notifymodel.h"

NotifyWidget::NotifyWidget(QWidget *parent, AbstractPersistence *database)
    : QWidget(parent)
{
    initView(database);
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *mainVBLayout = new QVBoxLayout;
    m_noNotify = new QLabel(tr("No system notifications"));

    m_noNotify->setAlignment(Qt::AlignCenter);
    m_noNotify->setVisible(false);

    mainVBLayout->addWidget(m_mainList);
    mainVBLayout->addWidget(m_noNotify);
    mainVBLayout->setMargin(0);
    setLayout(mainVBLayout);
}

void NotifyWidget::initView(AbstractPersistence *database)
{
    m_mainList = new NotifyListView(this);
    m_model = new NotifyModel(this,database, m_mainList);
    m_mainList->setAccessibleName("NotifyListView");
    m_mainList->setModel(m_model);
    m_mainList->setItemDelegate(new ItemDelegate(m_mainList ,m_model, this));
    m_mainList->setAutoFillBackground(false);
    m_mainList->viewport()->setAutoFillBackground(false);
    m_mainList->setFrameStyle(QFrame::NoFrame);
    m_mainList->setMouseTracking(true);
    m_mainList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mainList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mainList->setVerticalScrollMode(QListView::ScrollPerPixel);
    m_mainList->setContentsMargins(0, 0, 0, 0);
    m_mainList->setUpdatesEnabled(true);
    m_mainList->setSelectionMode(QListView::NoSelection);
    m_mainList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_mainList->setFocusPolicy(Qt::NoFocus);

    QPalette pa = m_mainList->palette();
    pa.setColor(QPalette::Highlight, Qt::transparent);
    m_mainList->setPalette(pa);

    connect(m_mainList, &NotifyListView::focusOnButton, this, &NotifyWidget::focusOnButton);
}

void NotifyWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    m_model->collapseData();
}

void NotifyWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
}
