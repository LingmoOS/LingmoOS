// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINTOOLWIDGET_H
#define MAINTOOLWIDGET_H
#include "toolbutton.h"

#include <QWidget>
#include <QObject>
#include <QHBoxLayout>

#include <DGuiApplicationHelper>
#include <DLabel>
#include <DStackedWidget>
#include <DImageButton>
#include <DIconButton>
#include <DPushButton>
#include <DMenu>
DWIDGET_USE_NAMESPACE
class MainToolWidget : public DWidget
{
    Q_OBJECT
public:
    MainToolWidget(DWidget *parent = nullptr);
signals:
    void signalCloseButtonClicked();
    void signalSaveButtonClicked();
protected:
    void initMainToolWidget();
private:
    ToolButton *m_closeButton;
    ToolButton *m_saveButton;
};

#endif // MAINTOOLWIDGET_H
