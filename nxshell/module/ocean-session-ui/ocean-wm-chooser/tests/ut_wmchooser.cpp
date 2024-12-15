// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#define protected public
#include "wmchooser.h"
#undef private
#undef protected

#include <QDir>
#include <QEvent>

#include <gtest/gtest.h>

class UT_WMChooser : public testing::Test
{

};

TEST_F(UT_WMChooser, coverageTest)
{
    WMChooser wmChooser;
    wmChooser.createParentDir("/usr/share/TstWMChooser/");
    QDir dir("/usr/share/TstWMChooser/");
    if (dir.exists())
        dir.rmdir("/usr/share/TstWMChooser/");

    wmChooser.setConfigPath("config.ini");
    wmChooser.saveConfig();
    QEvent event(QEvent::Leave);
    wmChooser.leaveEvent(&event);
}
