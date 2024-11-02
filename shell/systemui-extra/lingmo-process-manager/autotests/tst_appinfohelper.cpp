/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QtTest>
#include <QFile>
#include "appinfohelper.h"

class TestAppInfoHelper : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
    void test_case2();
    void test_case3();

private:
    void newFile(const QString &fileName, const QByteArray &content);
};

void TestAppInfoHelper::initTestCase()
{
    system("/usr/bin/mkdir -p /tmp/test");
}

void TestAppInfoHelper::cleanupTestCase()
{
    system("/usr/bin/rm -r /tmp/test");
}

void TestAppInfoHelper::test_case1()
{
    auto fileName = QUuid::createUuid().toString(QUuid::Id128);
    QString filePath1 = "/tmp/" + fileName;
    QString filePath2 = "/tmp/test/" + fileName;
    newFile(filePath1, "1234");
    newFile(filePath2, "1234");

    QVERIFY(appinfo_helper::isSameDesktopFile(
        filePath1.toStdString(), filePath2.toStdString()));
}

void TestAppInfoHelper::test_case2()
{
    auto fileName = QUuid::createUuid().toString(QUuid::Id128);
    QString filePath1 = "/tmp/" + fileName;
    QString filePath2 = "/tmp/test/" + fileName;
    newFile(filePath1, "1234");
    newFile(filePath2, "12345");

    QVERIFY(!appinfo_helper::isSameDesktopFile(
        filePath1.toStdString(), filePath2.toStdString()));
}

void TestAppInfoHelper::test_case3()
{
    auto fileName1 = QUuid::createUuid().toString(QUuid::Id128);
    auto fileName2 = QUuid::createUuid().toString(QUuid::Id128);
    QString filePath1 = "/tmp/" + fileName1;
    QString filePath2 = "/tmp/test/" + fileName2;
    newFile(filePath1, "1234");
    newFile(filePath2, "1234");

    QVERIFY(!appinfo_helper::isSameDesktopFile(
        filePath1.toStdString(), filePath2.toStdString()));
}

void TestAppInfoHelper::newFile(const QString &fileName, const QByteArray &content)
{
    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadWrite));
    file.write(content);
    file.flush();
    file.close();
}

QTEST_MAIN(TestAppInfoHelper)

#include "tst_appinfohelper.moc"
