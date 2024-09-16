// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menucontroller.h"

#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <map>
DWIDGET_USE_NAMESPACE

MenuController::MenuController(QObject *parent)
    : QObject(parent)
{
    initMenuController();
}

void MenuController::initMenuController()
{
    m_menu = new DMenu();
    m_menu->setFocusPolicy(Qt::StrongFocus);
    m_saveAct = new QAction(tr("Save"), this);
    connect(m_saveAct, &QAction::triggered, [ = ] {
        emit saveAction();
    });

    m_closeAct = new QAction(tr("Exit"), this);
    connect(m_closeAct, &QAction::triggered, [ = ] {
        emit closeAction();
    });
    m_menu->addAction(m_saveAct);
    m_menu->addSeparator();
    m_menu->addAction(m_closeAct);

    connect(m_menu, &DMenu::aboutToHide, this, [ = ] {
        emit menuNoFocus();
    });
}

void MenuController::showMenu(QPoint pos)
{
    m_menu->popup(pos);
}

void MenuController::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
}

MenuController::~MenuController()
{
    if (m_menu) {
        delete  m_menu;
        m_menu = nullptr;
    }
    if (m_saveAct) {
        delete  m_saveAct;
        m_saveAct = nullptr;
    }
    if (m_closeAct) {
        delete  m_closeAct;
        m_closeAct = nullptr;
    }
}
