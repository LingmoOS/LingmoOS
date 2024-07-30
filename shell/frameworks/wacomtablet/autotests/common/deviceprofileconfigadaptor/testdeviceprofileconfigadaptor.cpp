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
#include "common/deviceprofileconfigadaptor.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDir>
#include <QString>
#include <QTemporaryFile>

#include <QtTest>

using namespace Wacom;

/**
 * @file testdeviceprofileconfigadaptor.cpp
 *
 * @test UnitTest for the device profiles config adaptor
 */
class TestDeviceProfileConfigAdaptor : public QObject
{
    Q_OBJECT

private slots:
    void testConfig();
};

QTEST_MAIN(TestDeviceProfileConfigAdaptor)

void TestDeviceProfileConfigAdaptor::testConfig()
{
    QTemporaryFile tempFile(QDir::tempPath() + QDir::separator() + QLatin1String("testdeviceprofileconfigadaptorrc_XXXXXX"));
    QVERIFY(tempFile.open());
    tempFile.close();
    tempFile.setAutoRemove(true);

    KSharedConfig::Ptr config = KSharedConfig::openConfig(tempFile.fileName(), KConfig::SimpleConfig);
    QVERIFY(config);

    KConfigGroup configGroup = KConfigGroup(config, QLatin1String("DEVICE"));

    DeviceProfile writeProfile;
    CommonTestUtils::setValues(writeProfile);

    DeviceProfileConfigAdaptor writeAdaptor(writeProfile);
    writeAdaptor.saveConfig(configGroup);

    config->sync();

    DeviceProfile readProfile;
    readProfile.setDeviceType(DeviceType::Pad); // we have to set the type manually as only properties are copied atm
    DeviceProfileConfigAdaptor readAdaptor(readProfile);
    readAdaptor.loadConfig(configGroup);

    CommonTestUtils::assertValues(readProfile);
}

#include "testdeviceprofileconfigadaptor.moc"
