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

#include "../commontestutils.h"
#include "common/deviceprofile.h"
#include "common/tabletprofile.h"
#include "common/tabletprofileconfigadaptor.h"

#include <QDir>
#include <QString>
#include <QTemporaryFile>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QtTest>

using namespace Wacom;

/**
 * @file testtabletprofileconfigadaptor.cpp
 *
 * @test UnitTest for the profile config adaptor
 */
class TestTabletProfileConfigAdaptor : public QObject
{
    Q_OBJECT

private slots:
    void testConfig();
};

QTEST_MAIN(TestTabletProfileConfigAdaptor)

void TestTabletProfileConfigAdaptor::testConfig()
{
    QTemporaryFile tempFile(QDir::tempPath() + QDir::separator() + QLatin1String("testtabletprofileconfigadaptorrc_XXXXXX"));
    QVERIFY(tempFile.open());
    tempFile.close();
    tempFile.setAutoRemove(true);

    KSharedConfig::Ptr config = KSharedConfig::openConfig(tempFile.fileName(), KConfig::SimpleConfig);
    QVERIFY(config);

    TabletProfile writeTabletProfile(QLatin1String("TABLET"));
    KConfigGroup configGroup = KConfigGroup(config, writeTabletProfile.getName());

    DeviceProfile writeDeviceProfile1;
    DeviceType writeDeviceProfile1Type = DeviceType::Eraser;
    DeviceProfile writeDeviceProfile2;
    DeviceType writeDeviceProfile2Type = DeviceType::Stylus;
    CommonTestUtils::setValues(writeDeviceProfile1);
    CommonTestUtils::setValues(writeDeviceProfile2);
    writeDeviceProfile1.setDeviceType(writeDeviceProfile1Type);
    ;
    writeDeviceProfile2.setDeviceType(writeDeviceProfile2Type);

    writeTabletProfile.setDevice(writeDeviceProfile1);
    writeTabletProfile.setDevice(writeDeviceProfile2);

    TabletProfileConfigAdaptor writeAdaptor(writeTabletProfile);
    writeAdaptor.saveConfig(configGroup);

    config->sync();

    TabletProfile readTabletProfile;
    TabletProfileConfigAdaptor readAdaptor(readTabletProfile);
    readAdaptor.loadConfig(configGroup);

    QCOMPARE(readTabletProfile.getName(), writeTabletProfile.getName());
    QVERIFY(readTabletProfile.listDevices().size() == writeTabletProfile.listDevices().size());
    QVERIFY(readTabletProfile.hasDevice(writeDeviceProfile1Type));
    QVERIFY(readTabletProfile.hasDevice(writeDeviceProfile2Type));

    DeviceProfile readDeviceProfile1 = readTabletProfile.getDevice(writeDeviceProfile1Type);
    DeviceProfile readDeviceProfile2 = readTabletProfile.getDevice(writeDeviceProfile2Type);

    CommonTestUtils::assertValues(readDeviceProfile1, writeDeviceProfile1Type.key().toLatin1().constData());
    CommonTestUtils::assertValues(readDeviceProfile2, writeDeviceProfile2Type.key().toLatin1().constData());
}

#include "testtabletprofileconfigadaptor.moc"
