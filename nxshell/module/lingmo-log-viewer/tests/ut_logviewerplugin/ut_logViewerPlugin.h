// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGVIEWERPLUGIN_TEST_H
#define LOGVIEWERPLUGIN_TEST_H

#include "../liblogviewerplugin/src/plugins/logviewerplugin.h"
#include "../liblogviewerplugin/src/plugininterfaces/logviewerplugininterface.h"

#include <gtest/gtest.h>

#include <QObject>

class LogViewerPlugin_UT : public QObject, public testing::Test
{
    Q_OBJECT
public:
    LogViewerPlugin_UT();
    void SetUp(); //TEST跑之前会执行SetUp

    void TearDown(); //TEST跑完之后会执行TearDown

    LogViewerPlugin *m_instance = nullptr;
};

#endif // LOGVIEWERPLUGIN_TEST_H
