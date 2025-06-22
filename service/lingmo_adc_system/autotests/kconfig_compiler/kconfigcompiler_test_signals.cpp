/*
    SPDX-FileCopyrightText: 2014 Alexander Richardson <alex.richardson@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include "signals_test_no_singleton.h"
#include "signals_test_no_singleton_dpointer.h"
#include "signals_test_singleton.h"
#include "signals_test_singleton_dpointer.h"
#include "signals_test_singleton_itemaccessors.h"
#include <QDebug>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QtGlobal>
#include <QtTestGui>
#include <functional>

class KConfigCompiler_Test_Signals : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSetters();
    void testSetters_data();
    void testSetProperty();
    void testSetProperty_data();
    void initTestCase();
    void cleanupTestCase();
};

static SignalsTestNoSingleton *noSingleton;
static SignalsTestNoSingletonDpointer *noSingletonDpointer;

void KConfigCompiler_Test_Signals::initTestCase()
{
    // These tests do a lot quite a few I/O operations, speed that up by using a QTemporaryFile.
    // At least on Linux this is often a tmpfs which means only RAM operations
    QTemporaryFile *tempFile1 = new QTemporaryFile(this);
    QTemporaryFile *tempFile2 = new QTemporaryFile(this);
    QTemporaryFile *tempFile3 = new QTemporaryFile(this);
    QTemporaryFile *tempFile4 = new QTemporaryFile(this);
    QTemporaryFile *tempFile5 = new QTemporaryFile(this);
    QVERIFY(tempFile1->open());
    QVERIFY(tempFile2->open());
    QVERIFY(tempFile3->open());
    QVERIFY(tempFile4->open());
    QVERIFY(tempFile5->open());

    SignalsTestSingleton::instance(QFileInfo(*tempFile1).absoluteFilePath());
    SignalsTestSingletonDpointer::instance(QFileInfo(*tempFile2).absoluteFilePath());
    noSingleton = new SignalsTestNoSingleton(KSharedConfig::openConfig(QFileInfo(*tempFile3).absoluteFilePath(), KConfig::SimpleConfig));
    noSingletonDpointer = new SignalsTestNoSingletonDpointer(KSharedConfig::openConfig(QFileInfo(*tempFile4).absoluteFilePath(), KConfig::SimpleConfig));
    SignalsTestSingletonItemAccessors::instance(QFileInfo(*tempFile5).absoluteFilePath());
}

void KConfigCompiler_Test_Signals::cleanupTestCase()
{
    // ensure these instances are deleted before the temporary files are closed
    delete noSingleton;
    delete noSingletonDpointer;
    delete SignalsTestSingleton::self();
    delete SignalsTestSingletonDpointer::self();
    delete SignalsTestSingletonItemAccessors::self();
}

struct TestSettersArg {
    // default constructor required for Q_DECLARE_METATYPE
    TestSettersArg()
        : obj(nullptr)
    {
    }
    template<typename T>
    TestSettersArg(T *object)
        : obj(object)
    {
        // we can also call static methods using object->foo() so this works for all four cases
        getter = [object]() {
            return object->foo();
        };
        defaultGetter = [object]() {
            return object->defaultFooValue();
        };
        setter = [object](const QString &s) {
            object->setFoo(s);
        };
    }
    KCoreConfigSkeleton *obj;
    std::function<QString()> getter;
    std::function<QString()> defaultGetter;
    std::function<void(const QString &)> setter;
};

Q_DECLARE_METATYPE(TestSettersArg) // so that QFETCH works

void KConfigCompiler_Test_Signals::testSetters_data()
{
    QTest::addColumn<TestSettersArg>("params");
    QTest::newRow("singleton") << TestSettersArg(SignalsTestSingleton::self());
    QTest::newRow("singleton dpointer") << TestSettersArg(SignalsTestSingletonDpointer::self());
    QTest::newRow("non-singleton") << TestSettersArg(noSingleton);
    QTest::newRow("non-singleton dpointer") << TestSettersArg(noSingleton);
    QTest::newRow("singleton itemaccessors") << TestSettersArg(SignalsTestSingletonItemAccessors::self());
}

/** Ensure that a signal is emitted whenever the data is changed by using the generated setters */
void KConfigCompiler_Test_Signals::testSetters()
{
    QFETCH(TestSettersArg, params);
    const char *signal = SIGNAL(fooChanged(QString));
    const QString defaultValue = QStringLiteral("default");
    const QString changedValue = QStringLiteral("changed");

    // make sure we are in the default state
    params.obj->setDefaults();
    params.obj->save();

    QList<QVariant> args;
    QSignalSpy spy(params.obj, signal);
    QVERIFY2(spy.isValid(), signal);

    // change value via setter, should get signal
    QCOMPARE(params.getter(), defaultValue);
    QCOMPARE(defaultValue, params.defaultGetter());
    QCOMPARE(params.getter(), params.defaultGetter());
    QVERIFY(changedValue != params.getter());
    params.setter(changedValue);
    QCOMPARE(params.getter(), changedValue);
    QCOMPARE(spy.count(), 0); // should have no change yet, only after save()
    params.obj->save();
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.size(), 1);
    QCOMPARE(args[0].toString(), changedValue);

    // reset to default values via setDefaults()
    QVERIFY(params.getter() != params.defaultGetter());
    QVERIFY(params.getter() != defaultValue);
    QCOMPARE(params.getter(), changedValue);
    params.obj->setDefaults();
    QCOMPARE(params.getter(), params.defaultGetter());
    QCOMPARE(params.getter(), defaultValue);

    QCOMPARE(spy.count(), 0); // should have no change yet, only after save()
    params.obj->save();
    // TODO: This currently fails since setDefaults() does not yet cause emitting a signal
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.size(), 1);
    QCOMPARE(args[0].value<QString>(), defaultValue);
}

Q_DECLARE_METATYPE(KCoreConfigSkeleton *)

void KConfigCompiler_Test_Signals::testSetProperty_data()
{
    QTest::addColumn<KCoreConfigSkeleton *>("obj");
    QTest::newRow("singleton") << static_cast<KCoreConfigSkeleton *>(SignalsTestSingleton::self());
    QTest::newRow("singleton dpointer") << static_cast<KCoreConfigSkeleton *>(SignalsTestSingletonDpointer::self());
    QTest::newRow("non-singleton") << static_cast<KCoreConfigSkeleton *>(noSingleton);
    QTest::newRow("non-singleton dpointer") << static_cast<KCoreConfigSkeleton *>(noSingletonDpointer);
    QTest::newRow("singleton itemaccessors") << static_cast<KCoreConfigSkeleton *>(SignalsTestSingletonItemAccessors::self());
}

/** Test that the signal is emitted when modifying the values using the underlying KConfigSkeletonItem (bypassing the setters) */
void KConfigCompiler_Test_Signals::testSetProperty()
{
    QFETCH(KCoreConfigSkeleton *, obj);
    const char *signal = SIGNAL(fooChanged(QString));
    const QString propertyName = QStringLiteral("foo");
    const QString defaultValue = QStringLiteral("default");
    const QString newValue = QStringLiteral("changed");
    obj->setDefaults();
    obj->save();

    KConfigSkeletonItem *item = obj->findItem(propertyName);
    QVERIFY2(item, "Item must exist");
    QVERIFY2(!item->isImmutable(), "Item must not be immutable");
    QVERIFY2(!obj->isImmutable(propertyName), "Item must not be immutable");

    // listen for all expected signals
    QSignalSpy spy(obj, signal);
    QVERIFY2(spy.isValid(), signal);

    QVERIFY(item->isEqual(defaultValue));
    QVERIFY(!item->isEqual(newValue));

    item->setProperty(newValue); // change value now
    // should have no change yet, only after save()
    QCOMPARE(spy.count(), 0);
    obj->save();
    // now check for the signal emissions
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.size(), 1);
    QVERIFY(item->isEqual(args[0]));

    // now reset to default
    QVERIFY(!item->isEqual(defaultValue));
    item->setDefault();
    QVERIFY(item->isEqual(defaultValue));
    // should have no change yet, only after save()
    QCOMPARE(spy.count(), 0);
    obj->save();
    // now check for the signal emissions
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.size(), 1);
    QVERIFY(item->isEqual(args[0]));
}

QTEST_MAIN(KConfigCompiler_Test_Signals)

#include "kconfigcompiler_test_signals.moc"
