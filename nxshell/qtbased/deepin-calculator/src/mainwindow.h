// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dsettings.h"
#include "control/iconbutton.h"
#include "widgets/basicmodule.h"
#include "widgets/scientificmodule.h"
#include "widgets/programmodule.h"
#include "control/iconbutton.h"

#include <QStackedLayout>
#include <QMenu>
#include <DMainWindow>

#define ENABLE_SCIENTIFIC 1

DWIDGET_USE_NAMESPACE

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //switch to another mode and switch back
    void switchModeBack();

protected:
    void keyPressEvent(QKeyEvent *event);
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void initTheme();
    void initModule();
    void switchToSimpleMode();
    void switchToScientificMode();
    void switchToProgrammerMode();

private:
    DSettingsAlt *m_settings;
    QStackedLayout *m_mainLayout;
    DMenu *m_tbMenu;
    DMenu *m_modeshowmenu;
    QAction *m_simpleAction;
    QAction *m_scAction;
    QAction *m_programmerAction;
    QActionGroup *m_pActionGroup;

    BasicModule *m_basicModule = nullptr;
    scientificModule *m_scientificModule = nullptr;
    ProgramModule *m_programmerModule = nullptr;

    bool m_isinit = true;//初始化
    int programmerOldBase = 10;//切换前程序员计算器的进制
};

#endif
