// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drivericonwidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <DLabel>

#include <QIcon>

#include <gtest/gtest.h>


class UT_DriverIconWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        QIcon icon(QIcon::fromTheme("cautious"));
        QPixmap pic = icon.pixmap(80, 80);
        m_DriverIconWidgetPixMap = new DriverIconWidget(pic, "Warning", "The device will be unavailable after the driver uninstallation");
        m_DriverIconWidget = new DriverIconWidget("", "", nullptr);
    }
    void TearDown()
    {
        delete m_DriverIconWidgetPixMap;
        delete m_DriverIconWidget;
    }

    DriverIconWidget *m_DriverIconWidgetPixMap;
    DriverIconWidget *m_DriverIconWidget;
};
