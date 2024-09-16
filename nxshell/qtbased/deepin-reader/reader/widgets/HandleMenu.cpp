// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HandleMenu.h"
#include "DocSheet.h"

HandleMenu::HandleMenu(DWidget *parent)
    : DMenu(parent)
{
    initActions();
}

void HandleMenu::initActions()
{
    m_docSheet = nullptr;
    QActionGroup *actionGroup = new QActionGroup(this);

    m_textAction = new QAction(tr("Select Text"), this);
    connect(m_textAction, SIGNAL(triggered()), SLOT(onSelectText()));
    m_textAction->setObjectName("defaultshape");
    m_textAction->setCheckable(true);

    actionGroup->addAction(m_textAction);
    addAction(m_textAction);

    m_handAction = new QAction(tr("Hand Tool"), this);
    connect(m_handAction, SIGNAL(triggered()), SLOT(onHandTool()));
    m_handAction->setObjectName("handleshape");
    m_handAction->setCheckable(true);

    actionGroup->addAction(m_handAction);
    addAction(m_handAction);
}

void HandleMenu::onHandTool()
{
    if (m_docSheet)
        m_docSheet->setMouseShape(Dr::MouseShapeHand);
}

void HandleMenu::onSelectText()
{
    if (m_docSheet)
        m_docSheet->setMouseShape(Dr::MouseShapeNormal);
}

void HandleMenu::readCurDocParam(DocSheet *docSheet)
{
    m_docSheet = docSheet;
    if (!docSheet)
        return;
    if (m_docSheet->operation().mouseShape == Dr::MouseShapeNormal)
        m_textAction->setChecked(true);
    else if (m_docSheet->operation().mouseShape == Dr::MouseShapeHand)
        m_handAction->setChecked(true);
}
