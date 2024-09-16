// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menucontroller.h"
#include "../utils/configsettings.h"
#include "../utils/saveutils.h"
#include "../utils.h"

#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <map>
DWIDGET_USE_NAMESPACE
//const QSize MENU_ICON_SIZE = QSize(14, 14);

MenuController::MenuController(QObject *parent)
    : QObject(parent)
    , m_ration(1)
{
    m_menu = new DMenu();
    m_menu->setFocusPolicy(Qt::StrongFocus);
    //for test
//    m_menu->setStyle(QStyleFactory::create("dlight"));
    //for test

//    QIcon rectIcon;
//    rectIcon.addFile(":/image/menu_icons/rectangle-menu-norml.svg", MENU_ICON_SIZE,  QIcon::Normal);
//    rectIcon.addFile(":/image/menu_icons/rectangle-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    QAction *rectAct = new QAction(rectIcon, tr("Rectangle"), this);
//    connect(rectAct, &QAction::triggered, [ = ] {
//        emit shapePressed("rectangle");
//    });

//    QIcon ovalIcon;
//    ovalIcon.addFile(":/image/menu_icons/ellipse-menu-norml.svg", MENU_ICON_SIZE,  QIcon::Normal);
//    ovalIcon.addFile(":/image/menu_icons/ellipse-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    QAction *ovalAct = new QAction(ovalIcon, tr("Ellipse"), this);
//    connect(ovalAct, &QAction::triggered, [ = ] {
//        emit shapePressed("oval");
//    });

//    QIcon arrowIcon;
//    arrowIcon.addFile(":/image/menu_icons/arrow-menu-norml.svg", MENU_ICON_SIZE, QIcon::Normal);
//    arrowIcon.addFile(":/image/menu_icons/arrow-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    QAction *arrowAct = new QAction(arrowIcon, tr("Arrow"), this);
//    connect(arrowAct, &QAction::triggered, [ = ] {
//        emit shapePressed("arrow");
//    });

//    QIcon penIcon;
//    penIcon.addFile(":/image/menu_icons/line-menu-norml.svg", MENU_ICON_SIZE, QIcon::Normal);
//    penIcon.addFile(":/image/menu_icons/line-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    QAction *penAct = new QAction(penIcon, tr("Pencil"), this);
//    connect(penAct, &QAction::triggered, [ = ] {
//        emit shapePressed("line");
//    });

//    QIcon textIcon;
//    textIcon.addFile(":/image/menu_icons/text-menu-norml.svg", MENU_ICON_SIZE, QIcon::Normal);
//    textIcon.addFile(":/image/menu_icons/text-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    QAction *textAct = new QAction(textIcon, tr("Text"), this);
//    connect(textAct, &QAction::triggered, [ = ] {
//        emit shapePressed("text");
//    });

//    QIcon unDoIcon;
//    unDoIcon.addFile(":/image/menu_icons/undo-menu-normal.svg", MENU_ICON_SIZE, QIcon::Normal);
//    unDoIcon.addFile(":/image/menu_icons/undo-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    QAction *unDoAct = new QAction(unDoIcon, tr("Undo"), this);
    m_unDoAct = new QAction(tr("Undo"), this);
    Utils::setAccessibility(m_unDoAct, "menuUndo");
    connect(m_unDoAct, &QAction::triggered, [ = ] {
        emit unDoAction();
    });

    //QAction *saveAct = new QAction(tr("Save"), this);
    m_saveAct = new QAction(tr("Save"), this);
    Utils::setAccessibility(m_saveAct, "menuSave");
    connect(m_saveAct, &QAction::triggered, [ = ] {
        emit saveAction();
    });

    //QAction *closeAct = new QAction(tr("Exit"), this);
    m_closeAct = new QAction(tr("Exit"), this);
    Utils::setAccessibility(m_closeAct, "menuExit");
    connect(m_closeAct, &QAction::triggered, [ = ] {
        emit closeAction();
    });

//    m_menu->addAction(rectAct);
//    m_menu->addAction(ovalAct);
//    m_menu->addAction(arrowAct);
//    m_menu->addAction(penAct);
//    m_menu->addAction(textAct);
//    m_menu->addSeparator();
    m_menu->addAction(m_unDoAct);
    m_menu->addAction(m_saveAct);
    m_menu->addAction(m_closeAct);

    m_unDoAct->setEnabled(false);

//    QIcon saveIcon;
//    saveIcon.addFile(":/image/menu_icons/save-menu-norml.svg", MENU_ICON_SIZE, QIcon::Normal);
//    saveIcon.addFile(":/image/menu_icons/save-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    DMenu *saveMenu =  m_menu->addMenu(saveIcon, tr("Save"));

////    saveMenu->setStyle(QStyleFactory::create("dlight"));
//    QAction *saveAct1 = new QAction(tr("Save to desktop"), this);
//    QAction *saveAct2 = new QAction(tr("Autosave"), this);
//    QAction *saveAct3 = new QAction(tr("Save to specified folder"), this);
//    QAction *saveAct4 = new QAction(tr("Copy to clipboard"), this);
//    QAction *saveAct5 = new QAction(tr("Autosave and copy to clipboard"), this);

//    std::map<SaveAction, QAction *> actionMaps;
//    actionMaps[SaveAction::SaveToDesktop] = saveAct1;
//    actionMaps[SaveAction::AutoSave] = saveAct2;
//    actionMaps[SaveAction::SaveToSpecificDir] = saveAct3;
//    actionMaps[SaveAction::SaveToClipboard] = saveAct4;
//    actionMaps[SaveAction::SaveToAutoClipboard] = saveAct5;

//    SaveAction action = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();

//    for (auto it = actionMaps.begin(); it != actionMaps.end(); ++it) {
//        saveMenu->addAction(it->second);

//        connect(it->second, &QAction::triggered, this, [ = ] {
//            emit saveBtnPressed(action);
//        });

//        if (action == it->first) {
//            it->second->setCheckable(true);
//            it->second->setChecked(true);
//        }
//    }

//    QIcon exitIcon;
//    exitIcon.addFile(":/image/menu_icons/exit-menu-norml.svg", MENU_ICON_SIZE, QIcon::Normal);
//    exitIcon.addFile(":/image/menu_icons/exit-menu-hover.svg", MENU_ICON_SIZE, QIcon::Active);
//    QAction *closeAct = new QAction(exitIcon, tr("Exit"), this);
//    m_menu->addAction(closeAct);
//    connect(closeAct, &QAction::triggered, this, [ = ] {
//        emit shapePressed("close");
//    });

    connect(m_menu, &DMenu::aboutToHide, this, [ = ] {
        emit menuNoFocus();
    });
}

void MenuController::showMenu(QPoint pos)
{
    m_menu->popup(pos);
}

void MenuController::setUndoEnable(bool status)
{
    m_unDoAct->setEnabled(status);
}

void MenuController::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
}

MenuController::~MenuController()
{
    if(m_menu){
        delete  m_menu;
        m_menu = nullptr;
    }
    if(m_unDoAct){
        delete  m_unDoAct;
        m_unDoAct = nullptr;
    }
    if(m_saveAct){
        delete  m_saveAct;
        m_saveAct = nullptr;
    }
    if(m_closeAct){
        delete  m_closeAct;
        m_closeAct = nullptr;
    }
}
