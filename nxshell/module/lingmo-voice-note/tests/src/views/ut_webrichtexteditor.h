// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_WEBRICHTEXTEDITOR_H
#define UT_WEBRICHTEXTEDITOR_H

#include "webrichtexteditor.h"
#include "gtest/gtest.h"
#include <QTest>
#include <QObject>
#include <QWebEngineContextMenuData>

class UT_WebRichTextEditor : public QObject
    , public ::testing::Test
{
    Q_OBJECT
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

public:
    UT_WebRichTextEditor();
    WebRichTextEditor *m_web {nullptr};
    QWebEngineContextMenuData m_data;
    QWebEnginePage *stub_page();
    QWidget *stub_focusProxy();
    QWebEngineContextMenuData &stub_contextMenuData();
};

#endif // UT_WEBRICHTEXTEDITOR_H
