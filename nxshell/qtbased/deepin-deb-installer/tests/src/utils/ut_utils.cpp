// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../deb-installer/utils/utils.h"

#include <stub.h>
#include <DFontSizeManager>
#include <QProcess>
#include <DGuiApplicationHelper>
#include <QColor>
#include <QStorageInfo>
#include <QTemporaryFile>

DWIDGET_USE_NAMESPACE

QString stud_family()
{
    return "test";
}

TEST(Utils_Test, Utils_UT_loadFontFamilyByType)
{
    Stub stub;
    stub.set(ADDR(QFont, family), stud_family);
    Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    EXPECT_EQ("test", Utils::loadFontFamilyByType(Utils::SourceHanSansMedium));
}

TEST(Utils_Test, Utils_UT_loadFontBySizeAndWeight)
{
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);

    int fontsize = 14;
    QFont pkg_name_font = Utils::loadFontBySizeAndWeight(mediumFontFamily, fontsize, QFont::Medium);
    EXPECT_EQ(14, fontsize);
}
void util_setFont(const QFont &)
{
    return;
}
void util_bind(QWidget *, DFontSizeManager::SizeType, int)
{
    return;
}

TEST(Utils_Test, Utils_UT_bindFontBySizeAndWeight)
{
    Stub stub;
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    QWidget *widget = nullptr;

    stub.set(ADDR(QWidget, setFont), util_setFont);
    stub.set((void(DFontSizeManager::*)(QWidget *, DFontSizeManager::SizeType, int))ADDR(DFontSizeManager, bind), util_bind);

    Utils::bindFontBySizeAndWeight(widget, fontFamily, 14, QFont::Medium);
    EXPECT_EQ(nullptr, widget);
}

TEST(Utils_Test, Utils_UT_fromSpecialEncoding)
{
    ASSERT_STREQ(Utils::fromSpecialEncoding("name").toLocal8Bit(), "name");
}

TEST(Utils_Test, Utils_UT_Return_Digital_Verify_00)
{
    ASSERT_FALSE(Utils::Return_Digital_Verify("strfilepath", "strfilename"));
}

bool utils_exits()
{
    return true;
}

void util_setFilter(QDir::Filters filter)
{
    Q_UNUSED(filter);
    return;
}
QFileInfoList utils_entryInfoList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort)
{
    Q_UNUSED(filters);
    Q_UNUSED(sort);
    QList<QFileInfo> list;

    QFileInfo info;
    info.setFile("deepin-deb-verify");

    list << info;
    return list;
}

TEST(Utils_Test, Utils_UT_Return_Digital_Verify_01)
{
    Stub stub;
    stub.set((bool(QDir::*)() const)ADDR(QDir, exists), utils_exits);
    stub.set((QFileInfoList(QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList),
             utils_entryInfoList);
    stub.set((void(QDir::*)(QDir::Filters))ADDR(QDir, setFilter), util_setFilter);
    ASSERT_TRUE(Utils::Return_Digital_Verify("strfilepath", "deepin-deb-verify"));
}

void util_proc_start(const QString &program, const QStringList &arguments, QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
    return;
}

QByteArray util_readAllStandardOutput_success()
{
    return "[INFO] signature verified!";
}

QByteArray util_readAllStandardOutput_DebfileInexistence()
{
    return "cannot find signinfo in deb file";
}

QByteArray util_readAllStandardOutput_ExtractDebFail()
{
    return "extract deb_file failed!";
}

QByteArray util_readAllStandardOutput_DebVerifyFail()
{
    return "verify deb file failed!";
}

TEST(Utils_Test, Utils_UT_Digital_Verify_DebfileInexistence)
{
    Stub stub;
    stub.set((bool(QDir::*)() const)ADDR(QDir, exists), utils_exits);
    stub.set((QFileInfoList(QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList),
             utils_entryInfoList);
    stub.set((void(QDir::*)(QDir::Filters))ADDR(QDir, setFilter), util_setFilter);
    stub.set((void(QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             util_proc_start);

    stub.set(ADDR(QProcess, readAllStandardOutput), util_readAllStandardOutput_DebfileInexistence);
    stub.set(ADDR(QProcess, readAllStandardError), util_readAllStandardOutput_DebfileInexistence);
    ASSERT_EQ(Utils::Digital_Verify("strfilepath"), Utils::DebfileInexistence);
}

TEST(Utils_Test, Utils_UT_Digital_Verify_ExtractDebFail)
{
    Stub stub;
    stub.set((bool(QDir::*)() const)ADDR(QDir, exists), utils_exits);
    stub.set((QFileInfoList(QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList),
             utils_entryInfoList);
    stub.set((void(QDir::*)(QDir::Filters))ADDR(QDir, setFilter), util_setFilter);
    stub.set((void(QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             util_proc_start);

    stub.set(ADDR(QProcess, readAllStandardOutput), util_readAllStandardOutput_ExtractDebFail);
    stub.set(ADDR(QProcess, readAllStandardError), util_readAllStandardOutput_ExtractDebFail);
    ASSERT_EQ(Utils::Digital_Verify("strfilepath"), Utils::ExtractDebFail);
}

TEST(Utils_Test, Utils_UT_Digital_Verify_DebVerifyFail)
{
    Stub stub;
    stub.set((bool(QDir::*)() const)ADDR(QDir, exists), utils_exits);
    stub.set((QFileInfoList(QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList),
             utils_entryInfoList);
    stub.set((void(QDir::*)(QDir::Filters))ADDR(QDir, setFilter), util_setFilter);
    stub.set((void(QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             util_proc_start);

    stub.set(ADDR(QProcess, readAllStandardOutput), util_readAllStandardOutput_DebVerifyFail);
    stub.set(ADDR(QProcess, readAllStandardError), util_readAllStandardOutput_DebVerifyFail);
    ASSERT_EQ(Utils::Digital_Verify("strfilepath"), Utils::DebVerifyFail);
}

TEST(Utils_Test, Utils_UT_Digital_Verify)
{
    Stub stub;
    stub.set((bool(QDir::*)() const)ADDR(QDir, exists), utils_exits);
    stub.set((QFileInfoList(QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList),
             utils_entryInfoList);
    stub.set((void(QDir::*)(QDir::Filters))ADDR(QDir, setFilter), util_setFilter);
    stub.set((void(QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             util_proc_start);

    stub.set(ADDR(QProcess, readAllStandardOutput), util_readAllStandardOutput_success);
    stub.set(ADDR(QProcess, readAllStandardError), util_readAllStandardOutput_success);
    ASSERT_EQ(Utils::Digital_Verify("strfilepath"), Utils::VerifySuccess);
}

TEST(Utils_Test, Utils_UT_holdTextInRect)
{
    QFont font;
    QString info = "(Reading database ... 272597 files and directories currently installed.)";
    QString str = Utils::holdTextInRect(font, info, QSize(30, 600));
    ASSERT_TRUE(str.contains("\n"));
}

TEST(Utils_Test, Utils_UT_0012)
{
    QFont font;
    QString info = "(Reading database ... 272597 files and directories currently installed.)";
    QString str = Utils::holdTextInRect(font, info, 30);
    ASSERT_TRUE(str.contains("\n"));
}

TEST(Utils_Test, Utils_UT_0013)
{
    Utils util;
}

TEST(Utils_Test, Utils_UT_DebApplicationHelper)
{
    DebApplicationHelper *helper = DebApplicationHelper::instance();
    Q_UNUSED(helper);
}

TEST(Utils_Test, Utils_UT_standardPalette)
{
    DebApplicationHelper *helper = DebApplicationHelper::instance();
    helper->standardPalette(DGuiApplicationHelper::LightType);
}

void util_setPalette(const QPalette &)
{
    return;
}
bool util_setProperty(const char *, const QVariant &)
{
    return true;
}

TEST(Utils_Test, Utils_UT_setPalette)
{
    Stub stub;
    stub.set(ADDR(QWidget, setPalette), util_setPalette);
    stub.set(ADDR(QWidget, setProperty), util_setProperty);
    DebApplicationHelper *helper = DebApplicationHelper::instance();

    QWidget *w = nullptr;
    DPalette pa;
    helper->setPalette(w, pa);
}
void util_setAttribute(Qt::WidgetAttribute, bool)
{
    return;
}
TEST(Utils_Test, Utils_UT_resetPalette)
{
    Stub stub;
    stub.set(ADDR(QWidget, setProperty), util_setProperty);
    stub.set(ADDR(QWidget, setAttribute), util_setAttribute);
    DebApplicationHelper *helper = DebApplicationHelper::instance();

    QWidget *w = nullptr;
    DPalette pa;
    helper->resetPalette(w);
}

QString stub_storageinfo_device_gvfs()
{
    return "gvfsd-fuse";
}

TEST(Utils_Test, checkPackageReadable_deviceError_fail)
{
    Stub stub;
    stub.set(ADDR(QStorageInfo, device), stub_storageinfo_device_gvfs);

    EXPECT_FALSE(Utils::checkPackageReadable("/tmp"));
}

bool stub_file_isOpen_failed()
{
    return false;
}

TEST(Utils_Test, checkPackageReadable_readError_fail)
{
    Stub stub;
    stub.set(ADDR(QFile, isOpen), stub_file_isOpen_failed);

    QTemporaryFile tempFile;
    tempFile.open();
    tempFile.close();
    // This string is null before the QTemporaryFile is opened
    QString tmpFilePath = tempFile.fileName();

    EXPECT_FALSE(Utils::checkPackageReadable(tmpFilePath));
}

TEST(Utils_Test, checkPackageReadable_normal_success)
{
    QTemporaryFile tempFile;
    tempFile.open();
    tempFile.close();
    // This string is null before the QTemporaryFile is opened
    QString tmpFilePath = tempFile.fileName();

    EXPECT_TRUE(Utils::checkPackageReadable(tmpFilePath));
}
