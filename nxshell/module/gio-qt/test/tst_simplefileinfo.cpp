// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QString>
#include <QtTest>

#include <dgiofile.h>
#include <dgiofileinfo.h>

class DGioSimpleFileInfoTest : public QObject
{
    Q_OBJECT

public:
    DGioSimpleFileInfoTest();

private Q_SLOTS:
    void testCase_RegularFile();
};

DGioSimpleFileInfoTest::DGioSimpleFileInfoTest()
{
    //
}

void DGioSimpleFileInfoTest::testCase_RegularFile()
{
    QTemporaryFile tmpFile("test_RegularFile.txt");
    QVERIFY(tmpFile.open());
    QFileInfo tmpFileInfo(tmpFile);

    QScopedPointer<DGioFile> file(DGioFile::createFromPath(tmpFileInfo.absoluteFilePath()));
    QExplicitlySharedDataPointer<DGioFileInfo> fi = file->createFileInfo();
    QCOMPARE(file->basename(), tmpFileInfo.fileName());
    QCOMPARE(file->path(), tmpFileInfo.absoluteFilePath());
    QCOMPARE(fi->fileSize(), tmpFileInfo.size());
    qDebug() << fi->iconString() << fi->contentType();
    QVERIFY(fi->themedIconNames().contains("text-plain")); // is this check okay? better approach?
}

QTEST_APPLESS_MAIN(DGioSimpleFileInfoTest)

#include "tst_simplefileinfo.moc"
