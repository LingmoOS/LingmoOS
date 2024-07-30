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

#include "../tablethandlermock.h"

#include "kded/dbustabletservice.h"

#include "common/dbustabletinterface.h"
#include "common/tabletinformation.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testdbustabletservice.cpp
 *
 * @test UnitTest for ...
 */
class TestDBusTabletService : public QObject
{
    Q_OBJECT

public:
    void assertTabletInformation(const TabletInformation &expectedInformation) const;

public slots:
    void onProfileChanged(const QString &TabletId, const QString &profile);
    void onTabletAdded(const QString &TabletId);
    void onTabletRemoved(const QString &TabletId);

private slots:
    //! Run once before all tests.
    void initTestCase();

    void testListProfiles();
    void testOnTabletAdded();
    void testOnTabletRemoved();
    void testSetProfile();
    void testSetProperty();

    //! Run once after all tests.
    void cleanupTestCase();

private:
    TabletHandlerMock m_tabletHandlerMock;
    DBusTabletService *m_tabletService;

    QString m_profileWasChangedTo;
    bool m_tabletWasAdded;
    bool m_tabletWasRemoved;
};

QTEST_MAIN(TestDBusTabletService)

void TestDBusTabletService::assertTabletInformation(const TabletInformation &expectedInformation) const
{
    QDBusReply<QString> actualString;
    QDBusReply<bool> actualBool;

    // make sure the device list is equal
    QStringList expectedDeviceList = expectedInformation.getDeviceList();
    QDBusReply<QStringList> actualDeviceList = DBusTabletInterface::instance().getDeviceList(QLatin1String("TabletId"));
    QVERIFY(actualDeviceList.isValid());

    for (int i = 0; i < expectedDeviceList.size(); ++i) {
        QCOMPARE(actualDeviceList.value().at(i), expectedDeviceList.at(i));
    }

    // make sure the devices are equal
    foreach (const DeviceType &type, DeviceType::list()) {
        actualString = DBusTabletInterface::instance().getDeviceName(QLatin1String("TabletId"), type.key());
        QVERIFY(actualString.isValid());
        QCOMPARE(actualString.value(), expectedInformation.getDeviceName(type));
    }

    // compare tablet information
    foreach (const TabletInfo &info, TabletInfo::list()) {
        actualString = DBusTabletInterface::instance().getInformation(QLatin1String("TabletId"), info.key());
        QVERIFY(actualString.isValid());
        QCOMPARE(actualString.value(), expectedInformation.get(info));
    }

    // check pad buttons
    actualBool = DBusTabletInterface::instance().hasPadButtons(QLatin1String("TabletId"));
    QVERIFY(actualBool.isValid());
    QVERIFY(expectedInformation.hasButtons() == actualBool.value());

    // check availability
    actualBool = DBusTabletInterface::instance().isAvailable(QLatin1String("TabletId"));
    QVERIFY(actualBool.isValid());
    QVERIFY(expectedInformation.isAvailable() == actualBool.value());
}

void TestDBusTabletService::onProfileChanged(const QString &TabletId, const QString &profile)
{
    Q_UNUSED(TabletId)
    m_profileWasChangedTo = profile;
}

void TestDBusTabletService::onTabletAdded(const QString &TabletId)
{
    Q_UNUSED(TabletId)
    m_tabletWasAdded = true;
}

void TestDBusTabletService::onTabletRemoved(const QString &TabletId)
{
    Q_UNUSED(TabletId)
    m_tabletWasRemoved = true;
}

void TestDBusTabletService::initTestCase()
{
    m_tabletService = nullptr;
    m_profileWasChangedTo.clear();
    m_tabletWasAdded = false;
    m_tabletWasRemoved = false;

    // make sure the D-Bus service is not already registered
    DBusTabletInterface::instance().resetInterface();

    if (DBusTabletInterface::instance().isValid()) {
        QSKIP("D-Bus service already running! Please shut it down to run this test!", SkipAll);
        return;
    }

    // register D-Bus service
    m_tabletService = new DBusTabletService(m_tabletHandlerMock);

    // reset D-Bus client
    DBusTabletInterface::instance().resetInterface();
    ;

    // connect tablet service to us
    connect(m_tabletService, &DBusTabletService::profileChanged, this, &TestDBusTabletService::onProfileChanged);
    connect(m_tabletService, &DBusTabletService::tabletAdded, this, &TestDBusTabletService::onTabletAdded);
    connect(m_tabletService, &DBusTabletService::tabletRemoved, this, &TestDBusTabletService::onTabletRemoved);

    // connect tablet handler to tablet service
    connect(&m_tabletHandlerMock, &TabletHandlerMock::profileChanged, m_tabletService, &DBusTabletService::onProfileChanged);
    connect(&m_tabletHandlerMock, &TabletHandlerMock::tabletAdded, m_tabletService, &DBusTabletService::onTabletAdded);
    connect(&m_tabletHandlerMock, &TabletHandlerMock::tabletRemoved, m_tabletService, &DBusTabletService::onTabletRemoved);
}

void TestDBusTabletService::cleanupTestCase()
{
    if (m_tabletService) {
        delete m_tabletService;
        m_tabletService = nullptr;
    }
}

void TestDBusTabletService::testListProfiles()
{
    m_tabletHandlerMock.m_profiles.clear();

    QDBusReply<QStringList> profileList = DBusTabletInterface::instance().listProfiles(QLatin1String("TabletId"));
    QVERIFY(profileList.isValid());
    QVERIFY(profileList.value().isEmpty());

    m_tabletHandlerMock.m_profiles.append(QLatin1String("Test Profile 1"));
    m_tabletHandlerMock.m_profiles.append(QLatin1String("Test Profile 2"));

    profileList = DBusTabletInterface::instance().listProfiles(QLatin1String("TabletId"));

    QVERIFY(profileList.isValid());
    QCOMPARE(QLatin1String("Test Profile 1"), profileList.value().at(0));
    QCOMPARE(QLatin1String("Test Profile 2"), profileList.value().at(1));
}

void TestDBusTabletService::testOnTabletAdded()
{
    TabletInformation expectedInformation;

    foreach (const TabletInfo &tabletInfo, TabletInfo::list()) {
        expectedInformation.set(tabletInfo, tabletInfo.key());
    }

    expectedInformation.setAvailable(false); // this should be set to true automatically

    m_tabletWasAdded = false;
    m_tabletHandlerMock.emitTabletAdded(expectedInformation);

    QVERIFY(m_tabletWasAdded);

    expectedInformation.setAvailable(true);
    assertTabletInformation(expectedInformation);

    QDBusReply<bool> isAvail = DBusTabletInterface::instance().isAvailable(QLatin1String("TabletId"));
    QVERIFY(isAvail.isValid());
    QVERIFY(isAvail.value());
}

void TestDBusTabletService::testOnTabletRemoved()
{
    TabletInformation expectedInformation;
    expectedInformation.setAvailable(false);

    m_tabletWasRemoved = false;
    m_tabletHandlerMock.emitTabletRemoved(QLatin1String("TabletId"));
    assertTabletInformation(expectedInformation);
    QVERIFY(m_tabletWasRemoved);
}

void TestDBusTabletService::testSetProfile()
{
    QString expectedProfile;
    QDBusReply<QString> actualProfile;

    // set new profile and make sure a signal was emitted by D-Bus
    expectedProfile = QLatin1String("Test Profile");
    DBusTabletInterface::instance().setProfile(QLatin1String("TabletId"), expectedProfile);
    QCOMPARE(m_tabletHandlerMock.m_profile, expectedProfile);
    QCOMPARE(m_profileWasChangedTo, expectedProfile);

    // the new profile should also be returned by getProfile
    actualProfile = DBusTabletInterface::instance().getProfile(QLatin1String("TabletId"));
    QVERIFY(actualProfile.isValid());
    QCOMPARE(actualProfile.value(), expectedProfile);

    // when the tablet is removed, the profile should be reset
    // however the tablet handler should be unchanged and no signal should be emitted by D-Bus
    m_tabletHandlerMock.emitTabletRemoved(QLatin1String("TabletId"));
    actualProfile = DBusTabletInterface::instance().getProfile(QLatin1String("TabletId"));
    QVERIFY(actualProfile.isValid());
    QVERIFY(actualProfile.value().isEmpty());
    QCOMPARE(m_tabletHandlerMock.m_profile, expectedProfile);
    QCOMPARE(m_profileWasChangedTo, expectedProfile);

    // set another profile
    expectedProfile = QLatin1String("New Profile");
    DBusTabletInterface::instance().setProfile(QLatin1String("TabletId"), expectedProfile);
    QCOMPARE(m_tabletHandlerMock.m_profile, expectedProfile);
    QCOMPARE(m_profileWasChangedTo, expectedProfile);

    // the new profile should also be returned by getProfile
    actualProfile = DBusTabletInterface::instance().getProfile(QLatin1String("TabletId"));
    QVERIFY(actualProfile.isValid());
    QCOMPARE(actualProfile.value(), expectedProfile);

    // clear the profile manually
    expectedProfile.clear();
    DBusTabletInterface::instance().setProfile(QLatin1String("TabletId"), expectedProfile);
    QCOMPARE(m_tabletHandlerMock.m_profile, expectedProfile);
    QCOMPARE(m_profileWasChangedTo, expectedProfile);

    // the new profile should also be returned by getProfile
    actualProfile = DBusTabletInterface::instance().getProfile(QLatin1String("TabletId"));
    QVERIFY(actualProfile.isValid());
    QCOMPARE(actualProfile.value(), expectedProfile);
}

void TestDBusTabletService::testSetProperty()
{
    DeviceType expectedDevice = DeviceType::Stylus;
    Property expectedProperty = Property::Button1;
    QString expectedValue = QLatin1String("My Value");

    // set the property
    DBusTabletInterface::instance().setProperty(QLatin1String("TabletId"), expectedDevice.key(), expectedProperty.key(), expectedValue);

    QCOMPARE(m_tabletHandlerMock.m_deviceType, expectedDevice.key());
    QCOMPARE(m_tabletHandlerMock.m_property, expectedProperty.key());
    QCOMPARE(m_tabletHandlerMock.m_propertyValue, expectedValue);

    // try to get the property
    QDBusReply<QString> actualValue = DBusTabletInterface::instance().getProperty(QLatin1String("TabletId"), expectedDevice.key(), expectedProperty.key());

    QVERIFY(actualValue.isValid());
    QCOMPARE(actualValue.value(), expectedValue);
}

#include "testdbustabletservice.moc"
