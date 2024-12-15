// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCLEANGREETINGWIDGET_H
#define TRASHCLEANGREETINGWIDGET_H

#include "trashcleanconfigitem.h"

#include <DFrame>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <DSuggestButton>

#include <QList>
#include <QWidget>

class CleanerDBusAdaptorInterface;

class TrashCleanGreetingWidget : public Dtk::Widget::DFrame
{
    Q_OBJECT
public:
    explicit TrashCleanGreetingWidget(QWidget *parent = nullptr);

    ~TrashCleanGreetingWidget() { }

    inline const QList<int> &getScanConfig() { return m_scanConfigList; }

    void setGreeing(const QString &info);

Q_SIGNALS:
    void notifyStartScan();

public Q_SLOTS:
    void onItemStageChanged(int, int);
    void setPixmapByTheme(Dtk::Gui::DGuiApplicationHelper::ColorType themeType);

private:
    void initUI();
    void initConnection();

private:
    Dtk::Widget::DLabel *m_logo;
    Dtk::Widget::DLabel *m_title;
    Dtk::Widget::DLabel *m_greeting;
    TrashCleanConfigItem *m_sysItem;
    TrashCleanConfigItem *m_appItem;
    TrashCleanConfigItem *m_hisItem;
    TrashCleanConfigItem *m_cookiesItem;
    Dtk::Widget::DSuggestButton *m_scanBtn;
    QList<int> m_scanConfigList;

    // 应用界面工具对象，方便使用系统主题相关方法
    // DGuiApplicationHelper *m_guiHelper;
};

#endif // TRASHCLEANGREETINGWIDGET_H
