// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEMAINWINDOW_H
#define UT_VNOTEMAINWINDOW_H

#include "gtest/gtest.h"

#include <QTest>
#include <QObject>
#include <QWebChannel>
#include <QWebEnginePage>

class VNoteMainWindow;

class UT_VNoteMainWindow : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNoteMainWindow();
    VNoteMainWindow *m_mainWindow {nullptr};
    QWebEnginePage *stub_page();

protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif // UT_VNOTEMAINWINDOW_H
