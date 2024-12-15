// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QString>
#include <QtTest>

#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiomountoperation.h>
#include <dglibutils.h>

#include <giomm.h>

class DGioMatchGioEnumTest : public QObject
{
    Q_OBJECT

public:
    DGioMatchGioEnumTest();

private Q_SLOTS:
    void testCase_DGlibUtilsClass();
    void testCase_DGioFileClass();
    void testCase_DGioFileInfoClass();
    void testCase_DGioMountOperationClass();
};

DGioMatchGioEnumTest::DGioMatchGioEnumTest()
{
    //
}

void DGioMatchGioEnumTest::testCase_DGlibUtilsClass()
{
    QCOMPARE(DGlibFormatSizeFlag::FORMAT_SIZE_DEFAULT, Glib::FORMAT_SIZE_DEFAULT);
    QCOMPARE(DGlibFormatSizeFlag::FORMAT_SIZE_LONG_FORMAT, Glib::FORMAT_SIZE_LONG_FORMAT);
    QCOMPARE(DGlibFormatSizeFlag::FORMAT_SIZE_IEC_UNITS, Glib::FORMAT_SIZE_IEC_UNITS);
    QCOMPARE(DGlibFormatSizeFlag::FORMAT_SIZE_BITS, Glib::FORMAT_SIZE_BITS);
}

void DGioMatchGioEnumTest::testCase_DGioFileClass()
{
    QCOMPARE(DGioFileQueryInfoFlag::FILE_QUERY_INFO_NONE, Gio::FILE_QUERY_INFO_NONE);
    QCOMPARE(DGioFileQueryInfoFlag::FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, Gio::FILE_QUERY_INFO_NOFOLLOW_SYMLINKS);
}

void DGioMatchGioEnumTest::testCase_DGioFileInfoClass()
{
    QCOMPARE(DGioFileType::FILE_TYPE_NOT_KNOWN, Gio::FILE_TYPE_NOT_KNOWN);
    QCOMPARE(DGioFileType::FILE_TYPE_SYMBOLIC_LINK, Gio::FILE_TYPE_SYMBOLIC_LINK);
    QCOMPARE(DGioFileType::FILE_TYPE_MOUNTABLE, Gio::FILE_TYPE_MOUNTABLE);
}

void DGioMatchGioEnumTest::testCase_DGioMountOperationClass()
{
    QCOMPARE(DGioAskPasswordFlag::ASK_PASSWORD_NEED_PASSWORD, Gio::ASK_PASSWORD_NEED_PASSWORD);
    QCOMPARE(DGioAskPasswordFlag::ASK_PASSWORD_NEED_DOMAIN, Gio::ASK_PASSWORD_NEED_DOMAIN);
    QCOMPARE(DGioAskPasswordFlag::ASK_PASSWORD_ANONYMOUS_SUPPORTED, Gio::ASK_PASSWORD_ANONYMOUS_SUPPORTED);

    QCOMPARE(DGioMountOperationResult::MOUNT_OPERATION_ABORTED, Gio::MountOperationResult::MOUNT_OPERATION_ABORTED);
    QCOMPARE(DGioMountOperationResult::MOUNT_OPERATION_HANDLED, Gio::MountOperationResult::MOUNT_OPERATION_HANDLED);
    QCOMPARE(DGioMountOperationResult::MOUNT_OPERATION_UNHANDLED, Gio::MountOperationResult::MOUNT_OPERATION_UNHANDLED);
}

QTEST_APPLESS_MAIN(DGioMatchGioEnumTest)

#include "tst_matchgioenum.moc"
