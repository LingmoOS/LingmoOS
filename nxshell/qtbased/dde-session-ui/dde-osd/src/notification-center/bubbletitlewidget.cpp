// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubbletitlewidget.h"
#include "notification/bubbletool.h"
#include "notifylistview.h"

#include <QKeyEvent>
#include <QBoxLayout>
#include <QScroller>

#include <DFontSizeManager>
#include <DSysInfo>

BubbleTitleWidget::BubbleTitleWidget(NotifyModel *model, EntityPtr entity, QWidget *parent)
    : DWidget(parent),
      m_titleLabel(nullptr),
      m_model(model)
{
    this->setObjectName(entity->appName());
    m_titleLabel = new DLabel;
    m_titleLabel->setForegroundRole(QPalette::BrightText);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFont font = m_titleLabel->font();
    font.setBold(true);
    font.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(font);
    m_titleLabel->setText(BubbleTool::getDeepinAppName(entity->appName()));
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T4);

    m_closeButton = new DIconButton(DStyle::SP_CloseButton);
    m_closeButton->setObjectName(entity->appName() + "-CloseButton");
    m_closeButton->setFlat(true);
    m_closeButton->setIconSize(QSize(Notify::GroupButtonSize, Notify::GroupButtonSize));
    m_closeButton->setFixedSize(Notify::GroupButtonSize - 4, Notify::GroupButtonSize - 4);
    m_closeButton->setVisible(false);

    QHBoxLayout *head_Layout = new QHBoxLayout;
    head_Layout->setContentsMargins(10, 0, 0, 0);
    head_Layout->addWidget(m_titleLabel, Qt::AlignLeft);
    head_Layout->addStretch();
    head_Layout->addWidget(m_closeButton, Qt::AlignRight);
    setLayout(head_Layout);

    connect(m_closeButton, &DIconButton::clicked, this, [ = ] {
        m_model->removeAppGroup(entity->appName());
    });
}

void BubbleTitleWidget::setIndexRow(int row)
{
    m_indexRow = row;
}

void BubbleTitleWidget::enterEvent(QEvent *event)
{
    // QScroller::hasScroller用于判断listview是否处于滑动状态，滑动状态不触发paint相关操作，否则滑动动画异常
    // 欧拉此函数返回异常，且无触屏场景,不需要此判断
    if (Dtk::Core::DSysInfo::uosEditionType() == Dtk::Core::DSysInfo::UosEuler)
        m_closeButton->setVisible(true);
    if (!QScroller::hasScroller(m_view)) {
        m_closeButton->setVisible(true);
    }
    QWidget::enterEvent(event);
}

void BubbleTitleWidget::leaveEvent(QEvent *event)
{
    if (Dtk::Core::DSysInfo::uosEditionType() == Dtk::Core::DSysInfo::UosEuler)
        m_closeButton->setVisible(false);
    bool hasScroller = QScroller::hasScroller(m_view);
    if (!hasScroller) {
        m_closeButton->setVisible(false);
    } else {
        // 滚动结束,处理hover变化
        connect(QScroller::scroller(m_view), &QScroller::stateChanged, this, [this](const QScroller::State state){
            if (state == QScroller::Inactive) {
                m_closeButton->setVisible(false);
            }
        });
    }

    QWidget::enterEvent(event);
}

void BubbleTitleWidget::focusInEvent(QFocusEvent *event)
{
    m_closeButton->setVisible(true);
    QWidget::focusInEvent(event);
}

void BubbleTitleWidget::focusOutEvent(QFocusEvent *event)
{
    m_closeButton->setVisible(false);
    QWidget::focusOutEvent(event);
}

QList<QPointer<QWidget> > BubbleTitleWidget::bubbleElements()
{
    QList<QPointer<QWidget>> bubble_elements;
    bubble_elements.append(m_closeButton);
    return bubble_elements;
}

void BubbleTitleWidget::setParentView(NotifyListView *view)
{
    m_view = view;
}

int BubbleTitleWidget::bubbleTitleWidgetHeight()
{
    return qMax(QFontMetrics(DFontSizeManager::instance()->t4()).height(), BubbleTitleHeight);
}
