// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktopfilegenerator.h"
#include <gtest/gtest.h>
#include <QVariant>

TEST(DesktopFileGenerator, generate)
{
    QVariantMap map;
    map.insert("Type", QString{"Application"});
    map.insert("Name",
               QVariant::fromValue(QStringMap{{"default", "UserApp"}, {"zh_CN", "yonghuyingyong"}, {"en_US", "setApplication"}}));

    map.insert("Actions", QStringList{"one", "two"});
    map.insert("Exec",
               QVariant::fromValue(QStringMap{
                   {"default", "/usr/bin/exec"}, {"one", "/usr/bin/exec --type=one"}, {"two", "/usr/bin/exec --type=two"}}));
    map.insert("Icon", QVariant::fromValue(QStringMap{{"default", "default-icon"}, {"one", "one-icon"}, {"two", "two-icon"}}));
    map.insert("ActionName",
               QVariantMap{{"one", QVariant::fromValue(QStringMap{{"default", "oneName"}, {"zh_CN", "yi"}, {"en_US", "one"}})},
                           {"two", QVariant::fromValue(QStringMap{{"default", "twoname"}, {"zh_CN", "er"}, {"en_US", "two"}})}});
    map.insert("Version", 1.0);
    map.insert("Terminal", false);
    map.insert("MimeType", QStringList{"text/html", "text/xml", "application/xhtml+xml"});

    QString errMsg{"NO ERROR"};
    auto content = DesktopFileGenerator::generate(map, errMsg);
    EXPECT_EQ(errMsg.toStdString(), QString{"NO ERROR"}.toStdString());

    QString expect{R"([Desktop Entry]
Actions=one;two
Exec=/usr/bin/exec
Icon=default-icon
MimeType=text/html;text/xml;application/xhtml+xml
Name=UserApp
Name[en_US]=setApplication
Name[zh_CN]=yonghuyingyong
Terminal=false
Type=Application
Version=1
X-Deepin-CreateBy=dde-application-manager

[Desktop Action one]
Exec=/usr/bin/exec --type=one
Icon=one-icon
Name=oneName
Name[en_US]=one
Name[zh_CN]=yi

[Desktop Action two]
Exec=/usr/bin/exec --type=two
Icon=two-icon
Name=twoname
Name[en_US]=two
Name[zh_CN]=er

)"};
    EXPECT_EQ(expect.toStdString(), content.toStdString());
}
