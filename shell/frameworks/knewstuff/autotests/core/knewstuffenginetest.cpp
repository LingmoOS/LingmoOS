/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QSignalSpy>
#include <QTest>
#include <QtGlobal>

#include "enginebase.h"
#include "entry.h"
#include "provider.h"
#include "qtquick/quickengine.h"

using namespace KNSCore;

class EngineTest : public QObject
{
    Q_OBJECT
public:
    Engine *engine = nullptr;
    const QString dataDir = QStringLiteral(DATA_DIR);

private Q_SLOTS:
    void initTestCase();
    void testPropertiesReading();
    void testProviderFileLoading();
};

void EngineTest::initTestCase()
{
    engine = new Engine(this);
    engine->setConfigFile(dataDir + QLatin1String("enginetest.knsrc"));
    QVERIFY(engine->isValid());
    QCOMPARE(engine->busyState(), Engine::BusyOperation::Initializing);
    QSignalSpy providersLoaded(engine, &Engine::signalProvidersLoaded);
    QVERIFY(providersLoaded.wait());
    QCOMPARE(engine->busyState(), Engine::BusyState());
}

void EngineTest::testPropertiesReading()
{
    QCOMPARE(engine->name(), QStringLiteral("InstallCommands"));
    QCOMPARE(static_cast<EngineBase *>(engine)->categories(),
             QStringList({QStringLiteral("KDE Wallpaper 1920x1200"), QStringLiteral("KDE Wallpaper 1600x1200")}));
    QCOMPARE(engine->useLabel(), QStringLiteral("UseLabelTest"));
    QVERIFY(engine->hasAdoptionCommand());
    QCOMPARE(engine->contentWarningType(), EngineBase::ContentWarningType::Executables);
}

void EngineTest::testProviderFileLoading()
{
    const QString providerId = QUrl::fromLocalFile(dataDir + QLatin1String("entry.xml")).toString();
    QSharedPointer<Provider> provider = engine->provider(providerId);
    QVERIFY(provider);
    QCOMPARE(engine->defaultProvider(), provider);
    QVERIFY(engine->isValid());

    engine->setSearchTerm(QStringLiteral("Entry 4"));
    QSignalSpy spy(engine, &Engine::signalEntriesLoaded);
    QVERIFY(spy.wait());
    const QVariantList entries = spy.last().constFirst().toList(); // From last signal emission
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.first().value<KNSCore::Entry>().name(), QStringLiteral("Entry 4 (ghns included)"));
}

QTEST_MAIN(EngineTest)

#include "knewstuffenginetest.moc"
