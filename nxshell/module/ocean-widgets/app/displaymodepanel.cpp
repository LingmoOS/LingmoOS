// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displaymodepanel.h"
#include "widgethandler.h"
#include "widgetmanager.h"
#include "utils.h"
#include "button.h"

#include <QScrollArea>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCoreApplication>

#include <DIconButton>

WIDGETS_FRAME_BEGIN_NAMESPACE
DisplayModePanelCell::DisplayModePanelCell(Instance *instance, QWidget *parent)
    : InstancePanelCell(instance, parent)
{
    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
}

void DisplayModePanelCell::setView()
{
    layout()->addWidget(view());
}

DisplayModePanel::DisplayModePanel(WidgetManager *manager, QWidget *parent)
    : InstancePanel (manager, parent)
{
    setContentsMargins(UI::Display::leftMargin, UI::Display::topMargin, UI::Display::rightMargin, UI::Display::bottomMargin);
}

void DisplayModePanel::init()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(UI::defaultMargins);
    layout->setSpacing(0);
    m_views->setFixedWidth(width());

    layout->addWidget(m_views);

    auto button = new TransparentButton();
    button->setObjectName("edit-button");
    button->setText(qApp->translate("DisplayModePanel","edit widget"));
    button->setFixedSize(UI::Display::EditSize);
    connect(button, &QPushButton::clicked, this, &DisplayModePanel::editClicked);
    layout->addSpacing(UI::Ins::button2FlowPanelTopMargin);
    layout->addWidget(button, 0, Qt::AlignHCenter);
    layout->addSpacing(UI::Ins::button2FlowPanelTopMargin);

    layout->addStretch();
}

DisplayModePanelCell *DisplayModePanel::createWidget(Instance *instance)
{
    auto cell = new DisplayModePanelCell(instance, m_views);

    return cell;
}
WIDGETS_FRAME_END_NAMESPACE
