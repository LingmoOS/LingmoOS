// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include <QObject>
#include <QEvent>

WIDGETS_FRAME_BEGIN_NAMESPACE
class WidgetManager;
class MainView;
WIDGETS_FRAME_END_NAMESPACE
class WidgetsServer : public QObject {
    Q_OBJECT
public:
    explicit WidgetsServer(QObject *parent = nullptr);
    virtual ~WidgetsServer() override;

    bool registerService();

    void start();

public Q_SLOTS:
    void Toggle();
    void Show();
    void Hide();
    void SyncWidgets();

private:
    WIDGETS_FRAME_NAMESPACE::WidgetManager *m_manager;
    WIDGETS_FRAME_NAMESPACE::MainView *m_mainView = nullptr;
};
