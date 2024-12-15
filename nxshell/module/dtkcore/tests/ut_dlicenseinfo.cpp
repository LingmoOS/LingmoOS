// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <gtest/gtest.h>
#include <QString>
#include "dlicenseinfo.h"

DCORE_USE_NAMESPACE

TEST(ut_DLicenseInfo, testLicense)
{
    DLicenseInfo licenseInfo;
    QString jsonContent = u8R"([
                                {
                                    "name": "dtk",
                                    "version": "5.6.8",
                                    "copyright": "Copyright 2023 THe Uniontech Company Ltd. All rights reserved.",
                                    "license": "LGPLv3"
                                }
                                ]
                              )";
    licenseInfo.setLicenseSearchPath(":/data/");

    ASSERT_TRUE(licenseInfo.loadContent(jsonContent.toLatin1()));

    EXPECT_EQ(licenseInfo.componentInfos().count(), 1);
    ASSERT_TRUE(licenseInfo.loadFile(":/data/example-license.json"));
    EXPECT_EQ(licenseInfo.componentInfos().count(), 1);
    ASSERT_FALSE(licenseInfo.licenseContent("LGPLv3").isEmpty());
}
