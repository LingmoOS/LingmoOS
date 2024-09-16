// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "monitorindicator.h"
#include "recognizedialog.h"

#include <QApplication>
#include <QMimeData>

class UT_RecognizeDialog : public testing::Test
{
public:
    void SetUp() override
    {
        // monitor = new Monitor("org.deepin.dde.Display1", "/org/deepin/dde/Display1/1", QDBusConnection::sessionBus());
        // widget = new RecognizeDialog(monitor);
    }

    void TearDown() override
    {
//        delete widget;
//        widget = nullptr;
    }

public:
    Monitor *monitor = nullptr;
    RecognizeDialog *widget = nullptr;
};

TEST_F(UT_RecognizeDialog, coverageTest)
{

}
