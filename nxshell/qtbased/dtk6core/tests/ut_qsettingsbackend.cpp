// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include "settings/dsettings.h"
#include "settings/dsettingsoption.h"
#include "settings/dsettingsgroup.h"
#include "settings/backend/gsettingsbackend.h"
#include "settings/backend/qsettingbackend.h"

DCORE_USE_NAMESPACE


class ut_QSettingsBackend : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    QString jsonContent;
    QString iniContent;
};

void ut_QSettingsBackend::SetUp()
{
    QFile file("/tmp/test.ini");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    jsonContent = " { \"groups\": [{ "
                  " \"key\": \"base\", "
                  " \"name\": \"Basic settings\", "
                  " \"groups\": [{ "
                  " \"key\": \"open_action\", "
                  " \"name\": \"Open Action\", "
                  " \"options\": [{ "
                  " \"key\": \"alway_open_on_new\", "
                  " \"type\": \"checkbox\", "
                  " \"text\": \"Always Open On New Windows\", "
                  " \"default\": true "
                  " }]  "
                  " }] }]}";
    iniContent = "[Test] \n \
                  value=false  ";

    out << iniContent;
    file.close();
}

void ut_QSettingsBackend::TearDown()
{
    QFile file("/tmp/test.ini");
    if (file.exists())
        file.remove();
}

TEST_F(ut_QSettingsBackend, testQSettingsBackendKeys)
{
    DSettings tmpSetting;
    QSettingBackend qBackend("/tmp/test.ini");
    tmpSetting.setBackend(&qBackend);
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(!qKeys.isEmpty());
}

TEST_F(ut_QSettingsBackend, testQSettingsBackendGetOption)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QSettingBackend qBackend("/tmp/test.ini");
    scopeSettings->setBackend(&qBackend);
    scopeSettings->sync();
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(!qKeys.isEmpty());
    QVariant value = qBackend.getOption("Test");
    ASSERT_TRUE(!value.toBool());
}

TEST_F(ut_QSettingsBackend, testQSettingsBackendDoOption)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QSettingBackend qBackend("/tmp/test.ini");
    scopeSettings->setBackend(&qBackend);
    Q_EMIT qBackend.setOption("Test", true);

    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(!qKeys.isEmpty());
    QVariant value = qBackend.getOption("Test");
    ASSERT_TRUE(!value.toBool());

    // ensure `DSettings` is released before `SettingBackend` if `doSetOption` maybe execute.
    scopeSettings.reset();
}
