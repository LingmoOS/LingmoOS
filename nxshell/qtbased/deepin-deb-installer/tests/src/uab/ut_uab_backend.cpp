// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDebug>

#include "../stub.h"

#include "../deb-installer/uab/uab_backend.h"

class utDebBackend : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void utDebBackend::SetUp() { }

void utDebBackend::TearDown() { }

static QByteArray kUabJsonDataExample { "{"
                                        "    \"digest\": \"319c85...\","
                                        "    \"layers\": ["
                                        "        {"
                                        "            \"info\": {"
                                        "                \"kind\": \"runtime\""
                                        "            }"
                                        "        },"
                                        "        {"
                                        "            \"info\": {"
                                        "                \"arch\": ["
                                        "                    \"x86_64\""
                                        "                ],"
                                        "                \"base\": \"main:org.deepin.foundation/23.0.0/x86_64\","
                                        "                \"channel\": \"main\","
                                        "                \"command\": ["
                                        "                    \"dde-calendar\""
                                        "                ],"
                                        "                \"description\": \"calendar for deepin os.\n\","
                                        "                \"id\": \"org.dde.calendar\","
                                        "                \"kind\": \"app\","
                                        "                \"module\": \"binary\","
                                        "                \"name\": \"dde-calendar\","
                                        "                \"runtime\": \"main:org.deepin.Runtime/23.0.1/x86_64\","
                                        "                \"schema_version\": \"1.0\","
                                        "                \"size\": 51503113,"
                                        "                \"version\": \"5.1.2.1\""
                                        "            },"
                                        "            \"minified\": false"
                                        "        }"
                                        "    ]"
                                        "}" };

Uab::UabPkgInfo::Ptr createPtr(const QString &id, const QString &version)
{
    auto ptr = Uab::UabPkgInfo::Ptr::create();
    ptr->id = id;
    ptr->version = version;
    return ptr;
};

void initDataSet(QList<Uab::UabPkgInfo::Ptr> &pkgList)
{
    pkgList.append(createPtr("org.deepin.pkg1", "2.0.0"));
    pkgList.append(createPtr("com.deepin.pkg1", "1.0.2"));
    pkgList.append(createPtr("com.deepin.pkg1", "1.0.0"));
    pkgList.append(createPtr("com.deepin.pkg1", "1.0.1"));
    pkgList.append(createPtr("com.deepin.pkg2", "1.0.0"));
}

TEST_F(utDebBackend, initSuccess)
{
    Uab::UabBackend *insPtr = Uab::UabBackend::instance();
    Uab::UabBackend::backendProcess(insPtr);

    insPtr->dumpPackageList();
}

TEST_F(utDebBackend, sortPackagesNormalSuccess)
{
    QList<Uab::UabPkgInfo::Ptr> pkgList;
    initDataSet(pkgList);

    Uab::UabBackend::sortPackages(pkgList);

    EXPECT_EQ(QString("com.deepin.pkg1"), pkgList.first()->id);
    EXPECT_EQ(QString("1.0.2"), pkgList.first()->version);
    EXPECT_EQ(QString("com.deepin.pkg2"), pkgList.at(3)->id);
    EXPECT_EQ(QString("org.deepin.pkg1"), pkgList.last()->id);
}

TEST_F(utDebBackend, findPackageContainFind)
{
    Uab::UabBackend *insPtr = Uab::UabBackend::instance();
    insPtr->m_packageList.clear();

    initDataSet(insPtr->m_packageList);
    Uab::UabBackend::sortPackages(insPtr->m_packageList);

    auto findPtr = insPtr->findPackage("com.deepin.pkg1");
    ASSERT_FALSE(findPtr.isNull());
    EXPECT_EQ(findPtr->id, QString("com.deepin.pkg1"));
    EXPECT_EQ(findPtr->version, QString("1.0.2"));

    findPtr = insPtr->findPackage("com.deepin.pkg2");
    ASSERT_FALSE(findPtr.isNull());
    EXPECT_EQ(findPtr->id, QString("com.deepin.pkg2"));
    EXPECT_EQ(findPtr->version, QString("1.0.0"));

    findPtr = insPtr->findPackage("org.deepin.pkg1");
    ASSERT_FALSE(findPtr.isNull());
    EXPECT_EQ(findPtr->id, QString("org.deepin.pkg1"));
    EXPECT_EQ(findPtr->version, QString("2.0.0"));
}

TEST_F(utDebBackend, findPackageNotContainNotFind)
{
    Uab::UabBackend *insPtr = Uab::UabBackend::instance();
    insPtr->m_packageList.clear();

    EXPECT_TRUE(insPtr->findPackage("test").isNull());
}

TEST_F(utDebBackend, packageFromMetaJsonNormalSuccess)
{
    Uab::UabBackend *insPtr = Uab::UabBackend::instance();
    insPtr->m_supportArchSet.clear();
    insPtr->m_supportArchSet.insert("x86_64");

    auto uabPtr = insPtr->packageFromMetaJson(kUabJsonDataExample);

    ASSERT_FALSE(uabPtr.isNull());
    EXPECT_EQ(uabPtr->id, QString("org.dde.calendar"));
    EXPECT_EQ(uabPtr->appName, QString("dde-calendar"));
    EXPECT_EQ(uabPtr->version, QString("5.1.2.1"));
    QStringList cmpArch { "x86_64" };
    EXPECT_EQ(uabPtr->architecture, cmpArch);
}
