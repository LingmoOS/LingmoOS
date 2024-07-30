/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../kdedtestutils.h"
#include "../tabletbackendmock.h"

#include "common/screensinfo.h"
#include "common/tabletinformation.h"
#include "kded/tabletbackendfactory.h"
#include "kded/tablethandler.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testdbustabletservice.cpp
 *
 * @test UnitTest for ...
 * @todo use QSignalSpy?
 */
class TestTabletHandler : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void onNotify(const QString &eventId, const QString &title, const QString &message, bool suggestConfigure);
    void onProfileChanged(const QString &tabletID, const QString &profile);
    void onTabletAdded(const TabletInformation &info);
    void onTabletRemoved(const QString &tabletID);

private slots:
    void initTestCase();
    void test();
    void cleanupTestCase();

private:
    void testListProfiles();
    void testOnScreenRotated();
    void testOnTabletAdded();
    void testOnTabletRemoved();
    void testOnTogglePenMode();
    void testOnToggleTouch();
    void testSetProfile();
    void testSetProperty();

    QString m_notifyEventId;
    QString m_notifyTitle;
    QString m_notifyMessage;

    QString m_profileChanged;

    TabletInformation m_tabletAddedInformation;
    bool m_tabletAdded;
    bool m_tabletRemoved;

    TabletHandler *m_tabletHandler;
    TabletBackendMock *m_backendMock;
};

QTEST_MAIN(TestTabletHandler)

void TestTabletHandler::onNotify(const QString &eventId, const QString &title, const QString &message, bool suggestConfigure)
{
    Q_UNUSED(suggestConfigure)

    m_notifyEventId = eventId;
    m_notifyTitle = title;
    m_notifyMessage = message;
}

void TestTabletHandler::onProfileChanged(const QString &tabletID, const QString &profile)
{
    Q_UNUSED(tabletID)

    m_profileChanged = profile;
}

void TestTabletHandler::onTabletAdded(const TabletInformation &info)
{
    m_tabletRemoved = false;
    m_tabletAdded = true;
    m_tabletAddedInformation = info;
}

void TestTabletHandler::onTabletRemoved(const QString &tabletID)
{
    Q_UNUSED(tabletID)

    m_tabletAdded = false;
    m_tabletRemoved = true;
}

void TestTabletHandler::initTestCase()
{
    m_tabletAdded = false;
    m_tabletRemoved = false;
    m_backendMock = nullptr;

    TabletBackendFactory::setUnitTest(true);

    QString profilePath = KdedTestUtils::getAbsolutePath(QLatin1String("testtablethandler.profilesrc"));
    QString configPath = KdedTestUtils::getAbsolutePath(QLatin1String("testtablethandler.configrc"));
    m_tabletHandler = new TabletHandler(profilePath, configPath);

    connect(m_tabletHandler, &TabletHandler::notify, this, &TestTabletHandler::onNotify);
    connect(m_tabletHandler, &TabletHandler::profileChanged, this, &TestTabletHandler::onProfileChanged);
    connect(m_tabletHandler, &TabletHandler::tabletAdded, this, &TestTabletHandler::onTabletAdded);
    connect(m_tabletHandler, &TabletHandler::tabletRemoved, this, &TestTabletHandler::onTabletRemoved);
}

void TestTabletHandler::cleanupTestCase()
{
    delete m_tabletHandler;
}

void TestTabletHandler::test()
{
    // only one test method as the test has to be executed in a specific order
    testOnTabletAdded();

    testSetProperty();

    testListProfiles();

    testSetProfile();

    testOnTogglePenMode();

    testOnToggleTouch();

    testOnScreenRotated();

    testOnTabletRemoved();
}

void TestTabletHandler::testListProfiles()
{
    QStringList profiles = m_tabletHandler->listProfiles(QLatin1String("4321"));

    QCOMPARE(profiles.size(), 2);
    QVERIFY(profiles.contains(QLatin1String("default")));
    QVERIFY(profiles.contains(QLatin1String("test")));

    QWARN("testListProfiles(): PASSED!");
}

void TestTabletHandler::testOnScreenRotated()
{
    m_tabletHandler->onMapToScreen1();
    // reset screen rotation
    m_tabletHandler->setProperty(QLatin1String("4321"), DeviceType::Stylus, Property::Rotate, ScreenRotation::NONE.key());
    m_tabletHandler->setProperty(QLatin1String("4321"), DeviceType::Eraser, Property::Rotate, ScreenRotation::NONE.key());
    m_tabletHandler->setProperty(QLatin1String("4321"), DeviceType::Touch, Property::Rotate, ScreenRotation::NONE.key());

    // we need the test profile set as it has the required property
    QCOMPARE(m_profileChanged, QLatin1String("test"));

    // compare start parameters
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Eraser, Property::Rotate), ScreenRotation::NONE.key());
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Stylus, Property::Rotate), ScreenRotation::NONE.key());
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Touch, Property::Rotate), ScreenRotation::NONE.key());

    // rotate screen
    m_tabletHandler->onScreenRotated(ScreensInfo::getPrimaryScreenName(), Qt::InvertedLandscapeOrientation);

    // validate result
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Eraser, Property::Rotate), ScreenRotation::HALF.key());
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Stylus, Property::Rotate), ScreenRotation::HALF.key());
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Touch, Property::Rotate), ScreenRotation::HALF.key());

    QWARN("testOnScreenRotated(): PASSED!");
}

void TestTabletHandler::testOnTabletAdded()
{
    QVERIFY(!m_tabletAdded);
    QVERIFY(!m_tabletRemoved);

    // Do not initialize the factory yet so it can not return a backend.
    TabletInformation basicInfo;
    m_notifyEventId.clear();
    m_notifyMessage.clear();
    m_notifyTitle.clear();

    // if the tablet backend factory returns NULL adding a tablet should fail
    TabletBackendFactory::setUnitTest(true);
    TabletBackendFactory::setTabletBackendMock(NULL);

    m_tabletHandler->onTabletAdded(basicInfo);

    QVERIFY(!m_tabletAdded);
    QVERIFY(!m_tabletRemoved);
    QVERIFY(m_notifyEventId.isEmpty());
    QVERIFY(m_notifyMessage.isEmpty());
    QVERIFY(m_notifyTitle.isEmpty());
    QVERIFY(m_profileChanged.isEmpty());

    // create a valid backend for the factory to return
    m_backendMock = new TabletBackendMock();

    m_backendMock->m_tabletInformation.set(TabletInfo::TabletSerial, QLatin1String("123"));
    m_backendMock->m_tabletInformation.set(TabletInfo::CompanyId, QLatin1String("1234"));
    m_backendMock->m_tabletInformation.set(TabletInfo::CompanyName, QLatin1String("Company"));
    m_backendMock->m_tabletInformation.set(TabletInfo::TabletId, QLatin1String("4321"));
    m_backendMock->m_tabletInformation.set(TabletInfo::TabletModel, QLatin1String("Tablet Model"));
    m_backendMock->m_tabletInformation.set(TabletInfo::TabletName, QLatin1String("Bamboo Create"));
    m_backendMock->m_tabletInformation.set(TabletInfo::NumPadButtons, QLatin1String("4"));
    m_backendMock->m_tabletInformation.setAvailable(true);

    DeviceInformation devInfoEraser(DeviceType::Eraser, QLatin1String("Eraser Device"));
    DeviceInformation devInfoStylus(DeviceType::Stylus, QLatin1String("Stylus Device"));
    DeviceInformation devInfoTouch(DeviceType::Touch, QLatin1String("Touch Device"));

    m_backendMock->m_tabletInformation.setDevice(devInfoEraser);
    m_backendMock->m_tabletInformation.setDevice(devInfoStylus);
    m_backendMock->m_tabletInformation.setDevice(devInfoTouch);

    TabletBackendFactory::setTabletBackendMock(m_backendMock);

    // add a tablet
    m_tabletHandler->onTabletAdded(m_backendMock->getInformation());

    QVERIFY(!m_tabletRemoved);
    QVERIFY(m_tabletAdded);
    QVERIFY(!m_notifyEventId.isEmpty());
    QVERIFY(!m_notifyMessage.isEmpty());
    QVERIFY(!m_notifyTitle.isEmpty());
    QVERIFY(!m_profileChanged.isEmpty());
    QCOMPARE(m_profileChanged, QLatin1String("test"));
    QCOMPARE(m_backendMock->m_tabletProfile.getName(), QLatin1String("test"));

    m_tabletAdded = false;
    m_notifyEventId.clear();
    m_notifyMessage.clear();
    m_notifyTitle.clear();
    m_profileChanged.clear();

    // add the tablet again
    m_tabletHandler->onTabletAdded(m_backendMock->getInformation());

    QVERIFY(!m_tabletRemoved);
    QVERIFY(!m_tabletAdded);
    QVERIFY(m_notifyEventId.isEmpty());
    QVERIFY(m_notifyMessage.isEmpty());
    QVERIFY(m_notifyTitle.isEmpty());
    QVERIFY(m_profileChanged.isEmpty());

    QWARN("testOnTabletAdded(): PASSED!");

    // prepare for the next test
    m_tabletAdded = true;
}

void TestTabletHandler::testOnTabletRemoved()
{
    QVERIFY(!m_tabletRemoved);
    QVERIFY(m_tabletAdded);

    m_notifyEventId.clear();
    m_notifyMessage.clear();
    m_notifyTitle.clear();

    TabletInformation info;

    // removing a different tablet then the one handled by the backend should do nothing
    info.set(TabletInfo::TabletSerial, QLatin1String("12"));
    m_tabletHandler->onTabletRemoved(info);

    QVERIFY(!m_tabletRemoved);
    QVERIFY(m_tabletAdded);
    QVERIFY(m_notifyEventId.isEmpty());
    QVERIFY(m_notifyMessage.isEmpty());
    QVERIFY(m_notifyTitle.isEmpty());

    // remove the currently managed tablet
    // this should emit a signal and notify the user
    info.set(TabletInfo::TabletSerial, QLatin1String("123"));
    info.set(TabletInfo::TabletId, QLatin1String("4321"));
    m_tabletHandler->onTabletRemoved(info);

    QVERIFY(m_tabletRemoved);
    QVERIFY(!m_tabletAdded);
    QVERIFY(!m_notifyEventId.isEmpty());
    QVERIFY(!m_notifyMessage.isEmpty());
    QVERIFY(!m_notifyTitle.isEmpty());

    m_backendMock = NULL; // no longer valid, as tablet was removed

    // remove the device again.
    m_tabletRemoved = false;
    m_notifyEventId.clear();
    m_notifyMessage.clear();
    m_notifyTitle.clear();

    m_tabletHandler->onTabletRemoved(info);

    QVERIFY(!m_tabletRemoved);
    QVERIFY(!m_tabletAdded);
    QVERIFY(m_notifyEventId.isEmpty());
    QVERIFY(m_notifyMessage.isEmpty());
    QVERIFY(m_notifyTitle.isEmpty());

    QWARN("testOnTabletRemoved(): PASSED!");
}

void TestTabletHandler::testOnTogglePenMode()
{
    m_backendMock->setProperty(DeviceType::Stylus, Property::Mode, QLatin1String("Absolute"));
    m_backendMock->setProperty(DeviceType::Eraser, Property::Mode, QLatin1String("Absolute"));

    QCOMPARE(m_backendMock->getProperty(DeviceType::Eraser, Property::Mode), QLatin1String("Absolute"));
    QCOMPARE(m_backendMock->getProperty(DeviceType::Stylus, Property::Mode), QLatin1String("Absolute"));

    m_tabletHandler->onTogglePenMode();

    QCOMPARE(m_backendMock->getProperty(DeviceType::Eraser, Property::Mode), QLatin1String("relative"));
    QCOMPARE(m_backendMock->getProperty(DeviceType::Stylus, Property::Mode), QLatin1String("relative"));

    m_tabletHandler->onTogglePenMode();

    QCOMPARE(m_backendMock->getProperty(DeviceType::Eraser, Property::Mode), QLatin1String("absolute"));
    QCOMPARE(m_backendMock->getProperty(DeviceType::Stylus, Property::Mode), QLatin1String("absolute"));

    QWARN("testOnTogglePenMode(): PASSED!");
}

void TestTabletHandler::testOnToggleTouch()
{
    m_tabletHandler->setProperty(QLatin1String("4321"), DeviceType::Touch, Property::Touch, QLatin1String("off"));

    QCOMPARE(m_backendMock->getProperty(DeviceType::Touch, Property::Touch), QLatin1String("off"));
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Touch, Property::Touch), QLatin1String("off"));

    m_tabletHandler->onToggleTouch();

    QCOMPARE(m_backendMock->getProperty(DeviceType::Touch, Property::Touch), QLatin1String("on"));

    m_tabletHandler->onToggleTouch();

    QCOMPARE(m_backendMock->getProperty(DeviceType::Touch, Property::Touch), QLatin1String("off"));

    QWARN("testOnToggleTouch(): PASSED!");
}

void TestTabletHandler::testSetProfile()
{
    m_profileChanged.clear();
    m_notifyEventId.clear();
    m_notifyMessage.clear();
    m_notifyTitle.clear();

    // set invalid profile first
    m_tabletHandler->setProfile(QLatin1String("4321"), QLatin1String("InvalidProfile"));

    QVERIFY(!m_profileChanged.isEmpty());
    QCOMPARE(m_profileChanged, m_tabletHandler->listProfiles(QLatin1String("4321")).first());
    QVERIFY(!m_notifyEventId.isEmpty());
    QVERIFY(!m_notifyMessage.isEmpty());
    QVERIFY(!m_notifyTitle.isEmpty());

    // set a valid profile
    m_notifyEventId.clear();
    m_notifyMessage.clear();
    m_notifyTitle.clear();

    m_tabletHandler->setProfile(QLatin1String("4321"), QLatin1String("default"));

    QVERIFY(m_notifyEventId.isEmpty());
    QVERIFY(m_notifyMessage.isEmpty());
    QVERIFY(m_notifyTitle.isEmpty());
    QCOMPARE(m_profileChanged, QLatin1String("default"));
    QCOMPARE(m_backendMock->m_tabletProfile.getName(), QLatin1String("default"));

    // set the profile back to "test" so we can run this tests
    // multiple times in a row. Otherwise the test will fail as
    // cmake copies our test data only once.
    m_tabletHandler->setProfile(QLatin1String("4321"), QLatin1String("test"));

    QWARN("testSetProfile(): PASSED!");
}

void TestTabletHandler::testSetProperty()
{
    QVERIFY(m_tabletAdded);

    m_tabletHandler->setProperty(QLatin1String("4321"), DeviceType::Stylus, Property::Button1, Property::Button1.key());
    QCOMPARE(m_backendMock->getProperty(DeviceType::Stylus, Property::Button1), Property::Button1.key());
    QCOMPARE(m_tabletHandler->getProperty(QLatin1String("4321"), DeviceType::Stylus, Property::Button1), Property::Button1.key());

    QWARN("testSetProperty(): PASSED!");
}

#include "testtablethandler.moc"
