// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QHBoxLayout>
#include <QEvent>
#include <QSizePolicy>
#include <QGraphicsDropShadowEffect>

#include "boxlayoutwidget.h"
#include "groupbuttonwidget.h"
#include "toolbutton.h"
#include "drawboard.h"
#include "pagescene.h"
const int BUTTON_MARGIN = 3;
const int MAX_HEIGHT = 36;
GroupButtonWidget::GroupButtonWidget(DrawBoard *drawBoard, QWidget *parent): AttributeWgt(EGroupWgt, parent), m_drawBoard(drawBoard)
{
    //组合按钮
    groupButton = new DToolButton(nullptr);
    groupButton->setIcon(QIcon::fromTheme("group_normal"));
    groupButton->setToolTip(tr("Group"));
    setWgtAccesibleName(groupButton, "groupButton");
    groupButton->setMinimumSize(38, 30);
    groupButton->setIconSize(QSize(16, 16));
    groupButton->setFocusPolicy(Qt::NoFocus);

    //释放组合按钮
    unGroupButton = new DToolButton(nullptr);
    unGroupButton->setIcon(QIcon::fromTheme("ungroup_normal"));
    unGroupButton->setToolTip(tr("Ungroup"));
    setWgtAccesibleName(unGroupButton, "unGroupButton");
    unGroupButton->setMinimumSize(38, 30);
    unGroupButton->setIconSize(QSize(16, 16));
    unGroupButton->setFocusPolicy(Qt::NoFocus);

    m_titleLabel = new QLabel(tr("Group"), this);
    //m_titleLabel->setDisabled(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_titleLabel);

    BoxLayoutWidget *boxLayoutWidget = new BoxLayoutWidget(this);
    boxLayoutWidget->setMargins(BUTTON_MARGIN);
    boxLayoutWidget->setMaximumHeight(MAX_HEIGHT);
    boxLayoutWidget->addWidget(groupButton);
    boxLayoutWidget->addWidget(unGroupButton);

    mainLayout->addWidget(boxLayoutWidget);
    setLayout(mainLayout);
    addHSeparateLine();

    groupButton->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    unGroupButton->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    connect(groupButton, &DIconButton::clicked, this, [ = ]() {
        emit this->buttonClicked(true, false);
    });
    connect(unGroupButton, &DIconButton::clicked, this, [ = ]() {
        emit this->buttonClicked(false, true);
    });

    connect(m_drawBoard, qOverload<Page *>(&DrawBoard::currentPageChanged), this, [ = ](Page * p) {
        if (nullptr != p) {
            if (nullptr != m_currentScene) {
                disconnect(m_currentScene);
            }


            m_currentScene = p->scene();
            connect(m_currentScene, &PageScene::selectionChanged, this, [ = ] {
                updateButtonStatus();
            });
            updateButtonStatus();
        }
    });

}

void GroupButtonWidget::setGroupFlag(bool canGroup, bool canUngroup)
{
    groupButton->setEnabled(canGroup);
    unGroupButton->setEnabled(canUngroup);
    setEnabled(canGroup || canUngroup);
}

void GroupButtonWidget::updateButtonStatus()
{
    if (nullptr == m_currentScene) {
        return;
    }

    bool isGroup = m_currentScene->isGroupable(m_currentScene->selectedPageItems());
    bool isUngroup = m_currentScene->isUnGroupable(m_currentScene->selectedPageItems());
    setGroupFlag(isGroup, isUngroup);
}


