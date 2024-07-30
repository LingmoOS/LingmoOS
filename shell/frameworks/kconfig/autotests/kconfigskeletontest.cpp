/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigskeletontest.h"

#include <QFont>
#include <QtTestGui>
#include <kconfig.h>

QTEST_MAIN(KConfigSkeletonTest)

// clazy:excludeall=non-pod-global-static

static const bool s_default_setting1{false};
static const QColor s_default_setting2{1, 2, 3};
static const QString s_default_setting4{QStringLiteral("Hello World")};

static const bool s_write_setting1{true};
static const QColor s_write_setting2{3, 2, 1};
static const QString s_write_setting4{QStringLiteral("KDE")};

static QFont defaultSetting3()
{
    return QFont{QStringLiteral("helvetica"), 12};
}

static QFont writeSettings3()
{
    return QFont{QStringLiteral("helvetica"), 14};
}

void KConfigSkeletonTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KConfigSkeletonTest::init()
{
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/kconfigskeletontestrc"});
    s = new KConfigSkeleton(QStringLiteral("kconfigskeletontestrc"));
    s->setCurrentGroup(QStringLiteral("MyGroup"));
    itemBool = s->addItemBool(QStringLiteral("MySetting1"), mMyBool, s_default_setting1);
    s->addItemColor(QStringLiteral("MySetting2"), mMyColor, s_default_setting2);

    s->setCurrentGroup(QStringLiteral("MyOtherGroup"));
    s->addItemFont(QStringLiteral("MySetting3"), mMyFont, defaultSetting3());
    s->addItemString(QStringLiteral("MySetting4"), mMyString, s_default_setting4);

    QCOMPARE(mMyBool, s_default_setting1);
    QCOMPARE(mMyColor, s_default_setting2);
    QCOMPARE(mMyFont, defaultSetting3());
    QCOMPARE(mMyString, s_default_setting4);

    QVERIFY(s->isDefaults());
    QVERIFY(!s->isSaveNeeded());
}

void KConfigSkeletonTest::cleanup()
{
    delete s;
}

void KConfigSkeletonTest::testSimple()
{
    mMyBool = s_write_setting1;
    mMyColor = s_write_setting2;
    mMyFont = writeSettings3();
    mMyString = s_write_setting4;

    QVERIFY(s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->save();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    mMyBool = false;
    mMyColor = QColor();
    mMyString.clear();
    mMyFont = QFont();

    QVERIFY(s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->read();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    QCOMPARE(mMyBool, s_write_setting1);
    QCOMPARE(mMyColor, s_write_setting2);
    QCOMPARE(mMyFont, writeSettings3());
    QCOMPARE(mMyString, s_write_setting4);
}

void KConfigSkeletonTest::testRemoveItem()
{
    QVERIFY(s->findItem(QStringLiteral("MySetting1")));
    s->removeItem(QStringLiteral("MySetting1"));
    QVERIFY(!s->findItem(QStringLiteral("MySetting1")));
}

void KConfigSkeletonTest::testClear()
{
    QVERIFY(s->findItem(QStringLiteral("MySetting2")));
    QVERIFY(s->findItem(QStringLiteral("MySetting3")));
    QVERIFY(s->findItem(QStringLiteral("MySetting4")));

    s->clearItems();

    QVERIFY(!s->findItem(QStringLiteral("MySetting2")));
    QVERIFY(!s->findItem(QStringLiteral("MySetting3")));
    QVERIFY(!s->findItem(QStringLiteral("MySetting4")));
}

void KConfigSkeletonTest::testDefaults()
{
    mMyBool = s_write_setting1;
    mMyColor = s_write_setting2;
    mMyFont = writeSettings3();
    mMyString = s_write_setting4;

    QVERIFY(s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->save();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->setDefaults();

    QVERIFY(s->isSaveNeeded());
    QVERIFY(s->isDefaults());

    QCOMPARE(mMyBool, s_default_setting1);
    QCOMPARE(mMyColor, s_default_setting2);
    QCOMPARE(mMyFont, defaultSetting3());
    QCOMPARE(mMyString, s_default_setting4);

    s->save();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(s->isDefaults());
}

void KConfigSkeletonTest::testKConfigDirty()
{
    itemBool->setValue(true);
    itemBool->writeConfig(s->sharedConfig().data());
    QVERIFY(s->sharedConfig()->isDirty());
    s->save();
    QVERIFY(!s->sharedConfig()->isDirty());

    itemBool->setValue(false);
    itemBool->writeConfig(s->sharedConfig().data());
    QVERIFY(s->sharedConfig()->isDirty());
    s->save();
    QVERIFY(!s->sharedConfig()->isDirty());
}

void KConfigSkeletonTest::testSaveRead()
{
    itemBool->setValue(true);
    s->save();

    itemBool->setValue(false);
    s->save();

    mMyBool = true;

    s->read();
    QCOMPARE(mMyBool, false);
}

#include "moc_kconfigskeletontest.cpp"
