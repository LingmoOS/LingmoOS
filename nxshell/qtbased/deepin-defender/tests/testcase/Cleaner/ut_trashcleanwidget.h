// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_TRASHCLEANWIDGET_H
#define UT_TRASHCLEANWIDGET_H

#include "gtest/gtest.h"
#include <QObject>

class CleanerDBusAdaptorInterface;

class TrashCleanWidgetTest : public testing::Test
    , public QObject
{
public:
    TrashCleanWidgetTest();

public:
    CleanerDBusAdaptorInterface *m_dbusInterface;

protected:
    virtual void SetUp();
    virtual void TearDown();
};

#endif // UT_TRASHCLEANWIDGET_H
