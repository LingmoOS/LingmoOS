// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <gtest/gtest.h>

#include <QFile>
#include <QDebug>
#include <QRandomGenerator>

#include "lsysinfo.h"
#include "oceansktopentry.h"
#include "test_helper.hpp"

DCORE_USE_NAMESPACE

class ut_DSysInfo : public testing::Test
{
protected:
    void SetUp() override {
        if (QLatin1String(LSYSINFO_PREFIX).isEmpty()) {
            GTEST_SKIP_("LSYSINFO_PREFIX not defined...");
        }
    }
    void TearDown() override {

    }
};

TEST_F(ut_DSysInfo, testOsVersion)
{
    FileGuard guard("/tmp/etc/os-version");
    DDesktopEntry entry(guard.fileName());
    entry.setStringValue("UnionTech OS Desktop", "SystemName", "Version");
    entry.setStringValue("统信桌面操作系统", "SystemName[zh_CN]", "Version");
    entry.setStringValue("Desktop", "ProductType", "Version");
    entry.setStringValue("桌面", "ProductType[zh_CN]", "Version");
    entry.setStringValue("Professional", "EditionName", "Version");
    entry.setStringValue("专业版", "EditionName[zh_CN]", "Version");
    entry.setStringValue("20", "MajorVersion", "Version");
    entry.setStringValue("100A", "MinorVersion", "Version");
    entry.setStringValue("11Z18.107.109", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());

    ASSERT_TRUE(DSysInfo::uosSystemName(QLocale("C")) == "UnionTech OS Desktop");
    ASSERT_TRUE(DSysInfo::uosSystemName(QLocale("zh_CN")) == "统信桌面操作系统");
    ASSERT_TRUE(DSysInfo::uosProductTypeName(QLocale("zh_CN")) == "桌面");
    ASSERT_TRUE(DSysInfo::uosProductTypeName(QLocale("C")) == "Desktop");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("zh_CN")) == "专业版");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("C")) == "Professional");
    ASSERT_TRUE(DSysInfo::majorVersion() == "20");
    ASSERT_TRUE(DSysInfo::minorVersion() == "100A");
    ASSERT_TRUE(DSysInfo::buildVersion() == "107.109");

    // test minVersion.BC SP1….SP99
    for (int i = 0; i < 3; ++i) {
        int sp = QRandomGenerator::global()->generate() % 100;
        entry.setStringValue(QString("%1").arg(1001 + sp * 10), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::spVersion() == (sp ? QString("SP%1").arg(sp) : QString()));
    }

    // test minVersion.D udpate1~udpate9 updateA~udpateZ
    for (int i = 0; i < 10; ++i) {
        entry.setStringValue(QString("%1").arg(1000 + i), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == (i ? QString("update%1").arg(i) : QString()));
    }

    for (char c = 'A'; c <= 'Z'; ++c) {
        entry.setStringValue(QString("100").append(c), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == QString("update%1").arg(c));
    }

    // test incalide MinorVersion
    entry.setStringValue(QString("100?"), "MinorVersion", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::udpateVersion() == QString());
    // restore MinorVersion
    entry.setStringValue(QString("1000"), "MinorVersion", "Version");
    ASSERT_TRUE(entry.save());

    // test OsBuild.B == 1 && OsBuild.D = [1, 6]
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosDesktop);
    for (int i = 1; i <= 6; ++i) {
        entry.setStringValue(QString("%1").arg(11008.107 + i * 10), "OsBuild", "Version");
        ASSERT_TRUE(entry.save());
        switch (i) {
        case 1:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosProfessional);
            break;
        case 2:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosHome);
            break;
        case 4:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosMilitary);
            break;
        case 5:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosDeviceEdition);
            break;
        case 6:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEducation);
            break;
        default:
            break;
        }
    }

    // test OsBuild.B == 2 && OsBuild.D = [1, 5]
    entry.setStringValue("12018.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosServer);
    for (int i = 1; i <= 5; ++i) {
        entry.setStringValue(QString("%1").arg(12008.107 + i * 10), "OsBuild", "Version");
        ASSERT_TRUE(entry.save());
        switch (i) {
        case 1:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEnterprise);
            break;
        case 2:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEnterpriseC);
            break;
        case 3:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEuler);
            break;
        case 4:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosMilitaryS);
            break;
        case 5:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosDeviceEdition);
            break;
        default:
            break;
        }
    }

    // test OsBuild.B == 3
    entry.setStringValue("13018.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosDevice);
    ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEnterprise);

    // test invalid OsBuild.B
    entry.setStringValue("10018.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosTypeUnknown);

    // 社区版测试
    entry.setStringValue("Community", "EditionName", "Version");
    entry.setStringValue("社区版", "EditionName[zh_CN]", "Version");
    entry.setStringValue("21.1.2", "MinorVersion", "Version");
    entry.setStringValue("11038.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());

    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("zh_CN")) == "社区版");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("C")) == "Community");
    ASSERT_TRUE(DSysInfo::minorVersion() == "21.1.2");
    ASSERT_TRUE(DSysInfo::buildVersion() == "107");

    //社区版A_BC_D模式 test minVersion.BC SP1….SP99
    for (int i = 0; i < 3; ++i) {
        int sp = QRandomGenerator::global()->generate() % 100;
        entry.setStringValue(QString("%1").arg(1001 + sp * 10), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::spVersion() == (sp ? QString("SP%1").arg(sp) : QString()));
    }

    //社区版A_BC_D模式 test minVersion.D udpate1~udpate9 updateA~udpateZ
    for (int i = 0; i < 10; ++i) {
        entry.setStringValue(QString("%1").arg(1000 + i), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == (i ? QString("update%1").arg(i) : QString()));
    }

    auto dmax = [](int x, int y){
        return x > y ? x : y;
    };
    //社区版A_B_C模式 test minVersion.BC SP1….SP99
    const QString &defalutSP("21.%1");
    for (int i = 1; i < 3; ++i) {
        int sp = dmax(QRandomGenerator::global()->generate() % 100, 1);
        entry.setStringValue(defalutSP.arg(sp), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::spVersion() == QString("SP%1").arg(sp));
    }

    //社区版A_B_C模式 test minVersion.D udpate1~udpate9 updateA~udpateZ
    const QString &defalutUpdate("21.1.%1");
    for (int i = 1; i < 3; ++i) {
        int sp = dmax(QRandomGenerator::global()->generate() % 100, 1);
        entry.setStringValue(defalutUpdate.arg(sp), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == QString("update%1").arg(sp));
    }

    // 家庭版测试
    entry.setStringValue("Home", "EditionName", "Version");
    entry.setStringValue("家庭版", "EditionName[zh_CN]", "Version");
    entry.setStringValue("21.0", "MinorVersion", "Version");
    entry.setStringValue("11078.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());

    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("zh_CN")) == "家庭版");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("C")) == "Home");
    ASSERT_TRUE(DSysInfo::minorVersion() == "21.0");
    ASSERT_TRUE(DSysInfo::buildVersion() == "107");
    ASSERT_TRUE(DSysInfo::spVersion() == QStringLiteral(""));
    ASSERT_TRUE(DSysInfo::udpateVersion() == QStringLiteral(""));
}

TEST_F(ut_DSysInfo, testdistributionInfo)
{
    FileGuard fg("/tmp/share/lingmo/distribution.info");
    DDesktopEntry entry(fg.fileName());
    entry.setStringValue("Lingmo", "Name", "Distribution");
    entry.setStringValue("www.lingmo.org", "WebsiteName", "Distribution");
    entry.setStringValue("https://www.lingmo.org", "Website", "Distribution");
    entry.setStringValue("Logo.svg", "Logo", "Distribution");
    entry.setStringValue("LogoLight.svg", "LogoLight", "Distribution");
    entry.setStringValue("LogoTransparent.svg", "LogoTransparent", "Distribution");

    entry.setStringValue("Lingmo-Manufacturer", "Name", "Manufacturer");
    entry.setStringValue("Lingmo-Distributor", "Name", "Distributor");
    ASSERT_TRUE(entry.save());

    EnvGuard guard;
    guard.set("XDG_DATA_HOME", "/tmp/share");
    ASSERT_EQ(DSysInfo::distributionInfoPath(), fg.fileName());
    auto website = DSysInfo::distributionOrgWebsite();
    ASSERT_EQ(website.first, "www.lingmo.org");
    ASSERT_EQ(website.second, "https://www.lingmo.org");
    ASSERT_EQ(DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Normal), "Logo.svg");
    ASSERT_EQ(DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Light), "LogoLight.svg");
    ASSERT_EQ(DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Symbolic), ""); // not set
    ASSERT_EQ(DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Transparent), "LogoTransparent.svg");

    ASSERT_EQ(DSysInfo::distributionOrgName(DSysInfo::Distribution), "Lingmo");
    ASSERT_EQ(DSysInfo::distributionOrgName(DSysInfo::Distributor), "Lingmo-Distributor");
    ASSERT_EQ(DSysInfo::distributionOrgName(DSysInfo::Manufacturer), "Lingmo-Manufacturer");

    guard.restore();
}

TEST_F(ut_DSysInfo, osRelease)
{
    FileGuard fg("/tmp/etc/os-release");
    DDesktopEntry entry(fg.fileName());
    entry.setStringValue("Lingmo", "ID", "Release");
    entry.setStringValue("20.9", "VERSION_ID", "Release");
    entry.setStringValue("Lingmo 20.9", "PRETTY_NAME", "Release");
    ASSERT_TRUE(entry.save());

    ASSERT_EQ(DSysInfo::operatingSystemName(), "Lingmo 20.9");
    ASSERT_EQ(DSysInfo::productType(), DSysInfo::Lingmo);
    ASSERT_EQ(DSysInfo::productTypeString(), "Lingmo");
    ASSERT_EQ(DSysInfo::productVersion(), "20.9");

    // isLingmo
    ASSERT_TRUE(DSysInfo::isLingmo());
    entry.setStringValue("Uos", "ID", "Release");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::isLingmo());

    entry.setStringValue("arch", "ID", "Release");
    ASSERT_TRUE(entry.save());
    ASSERT_FALSE(DSysInfo::isLingmo());

    QString types[] = {"UnknownType", "Lingmo", "Arch", "CentOS", "Debian",
                       "Fedora", "LinuxMint", "Manjaro", "openSUSE","SailfishOS",
                       "Ubuntu", "Uos", "Gentoo", "NixOS"};
    for (int i = DSysInfo::UnknownType; i <= DSysInfo::NixOS; ++i) {
        entry.setStringValue(types[i], "ID", "Release");
        ASSERT_TRUE(entry.save());
        ASSERT_EQ(DSysInfo::productType(), DSysInfo::ProductType(i));
    }
}

TEST_F(ut_DSysInfo, isOCEAN)
{
    FileGuard fg("/tmp/etc/os-release");
    DDesktopEntry entry(fg.fileName());
    entry.setStringValue("Lingmo 20.9", "PRETTY_NAME", "Release");
    entry.setStringValue("Lingmo", "ID", "Release");
    entry.setStringValue("20.9", "VERSION_ID", "Release");
    ASSERT_TRUE(entry.save());

    FileGuard fg2("/tmp/etc/lingmo-version");
    DDesktopEntry entry2(fg2.fileName());
    entry2.setStringValue("20.9", "Version", "Release");
    entry2.setStringValue("Desktop", "Type", "Release");
    ASSERT_TRUE(entry2.save());

    // isLingmo && lingmoType valid
    ASSERT_TRUE(DSysInfo::isLingmo());
    ASSERT_TRUE(DSysInfo::isOCEAN());

    // isLingmo but lingmoType unknow
    entry2.setStringValue("Unknown", "Type", "Release");
    ASSERT_TRUE(entry2.save());
    ASSERT_TRUE(DSysInfo::isLingmo());
    ASSERT_FALSE(DSysInfo::isOCEAN());

    // !isLingmo && XDG_SESSION_DESKTOP == ocean or lingmo
    {
        entry.setStringValue("Unknown", "ID", "Release");
        ASSERT_TRUE(entry.save());
        ASSERT_FALSE(DSysInfo::isLingmo());
        EnvGuard guard;

        guard.set("XDG_SESSION_DESKTOP", "ocean");
        ASSERT_TRUE(DSysInfo::isOCEAN());
        guard.restore();

        guard.set("XDG_SESSION_DESKTOP", "lingmo");
        ASSERT_TRUE(DSysInfo::isOCEAN());
        guard.restore();

        guard.set("XDG_SESSION_DESKTOP", "Unknown");
        ASSERT_FALSE(DSysInfo::isOCEAN());
        guard.restore();
    }

}

TEST_F(ut_DSysInfo, lingmoVersion)
{
    FileGuard fg("/tmp/etc/lingmo-version");
    DDesktopEntry entry(fg.fileName());
    entry.setStringValue("20.9", "Version", "Release");
    entry.setStringValue("Desktop", "Type", "Release");
    entry.setStringValue("社区版", "Type[zh_CN]", "Release");
    entry.setStringValue("Y2020E0001", "Edition", "Release");
    entry.setStringValue("Y2020CR001", "Copyright", "Release");
    entry.setStringValue("build1", "Buildid", "Addition");
    //entry.setStringValue("", "Milestone", "Addition");
    ASSERT_TRUE(entry.save());

    ASSERT_EQ(DSysInfo::lingmoTypeDisplayName(QLocale("C")), "Desktop");
    ASSERT_EQ(DSysInfo::lingmoTypeDisplayName(QLocale("zh_CN")), "社区版");
    ASSERT_EQ(DSysInfo::lingmoVersion(), "20.9");
    ASSERT_EQ(DSysInfo::lingmoEdition(), "Y2020E0001");
    ASSERT_EQ(DSysInfo::lingmoCopyright(), "Y2020CR001");

    qInfo() << "DSysInfo::lingmoType()" << DSysInfo::lingmoType();
    ASSERT_EQ(DSysInfo::lingmoType(), DSysInfo::LingmoDesktop);
    {
        // isCommunityEdition Not Uos
        FileGuard fg("/tmp/etc/os-release");
        DDesktopEntry entry(fg.fileName());
        entry.setStringValue("Lingmo", "ID", "Release");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::isCommunityEdition());

        entry.setStringValue("Uos", "ID", "Release");
        ASSERT_TRUE(entry.save());
        ASSERT_FALSE(DSysInfo::isCommunityEdition());
    }

    entry.setStringValue("Professional", "Type", "Release");
    ASSERT_TRUE(entry.save());
    ASSERT_EQ(DSysInfo::lingmoType(), DSysInfo::LingmoProfessional);
    ASSERT_FALSE(DSysInfo::isCommunityEdition());

    entry.setStringValue("Server", "Type", "Release");
    ASSERT_TRUE(entry.save());
    ASSERT_EQ(DSysInfo::lingmoType(), DSysInfo::LingmoServer);
    ASSERT_FALSE(DSysInfo::isCommunityEdition());

    entry.setStringValue("Personal", "Type", "Release");
    ASSERT_TRUE(entry.save());
    ASSERT_EQ(DSysInfo::lingmoType(), DSysInfo::LingmoPersonal);
    ASSERT_FALSE(DSysInfo::isCommunityEdition());
}

TEST_F(ut_DSysInfo, other)
{
    qDebug() << DSysInfo::computerName();
    qDebug() << DSysInfo::memoryInstalledSize();
    qDebug() << DSysInfo::memoryTotalSize();
    qDebug() << DSysInfo::systemDiskSize();
}
