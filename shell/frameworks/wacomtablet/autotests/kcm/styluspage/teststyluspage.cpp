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

#include "../profilemanagementmocks.h"
#include "kcmodule/styluspagewidget.h"

#include "deviceprofile.h"
#include "deviceprofiledefaults.h"
#include "deviceproperty.h"
#include "screenrotation.h"

#include "ui_styluspagewidget.h"

#include <QtTest>

#include <QSlider>

using namespace Wacom;

class TestStylusPageWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testStylusSettingsPersistency();
    void testSettingsStylus();

private:
    ProfileManagementIntegrityChecker p;
    StylusPageWidget *_testObject;
};

void TestStylusPageWidget::initTestCase()
{
    p._savedProfiles.clear();
    p._presetProfiles.clear();
    _testObject = new StylusPageWidget();
}

void TestStylusPageWidget::cleanupTestCase()
{
    delete _testObject;
}

void TestStylusPageWidget::testStylusSettingsPersistency()
{
    QVERIFY(_testObject != nullptr);

    DeviceProfile stylus;
    DeviceProfile eraser;

    stylus.setDeviceType(DeviceType::Stylus);
    eraser.setDeviceType(DeviceType::Eraser);
    setupDefaultStylus(stylus);
    setupDefaultStylus(eraser);

    p._presetProfiles[DeviceType::Stylus] = stylus;
    p._presetProfiles[DeviceType::Eraser] = eraser;

    _testObject->loadFromProfile(p);
    _testObject->saveToProfile(p);

    for (const DeviceProperty &property : DeviceProperty::list()) {
        // qDebug() << "Comparing" << property.key();
        QCOMPARE(p._savedProfiles[DeviceType::Stylus].getProperty(property.id()), p._presetProfiles[DeviceType::Stylus].getProperty(property.id()));
        QCOMPARE(p._savedProfiles[DeviceType::Eraser].getProperty(property.id()), p._presetProfiles[DeviceType::Eraser].getProperty(property.id()));
    }
}

void TestStylusPageWidget::testSettingsStylus()
{
    DeviceProfile stylus;
    stylus.setDeviceType(DeviceType::Stylus);
    setupDefaultStylus(stylus);
    stylus.setProperty(Property::RawSample, QLatin1String("15"));

    p._presetProfiles[DeviceType::Stylus] = stylus;

    _testObject->loadFromProfile(p);

    _testObject->findChild<QSlider *>(QLatin1String("horizontalSliderRawSample"))->setValue(15);

    _testObject->saveToProfile(p);
    for (const DeviceProperty &property : DeviceProperty::list()) {
        // qDebug() << "Comparing" << property.key();
        QCOMPARE(p._savedProfiles[DeviceType::Stylus].getProperty(property.id()), p._presetProfiles[DeviceType::Stylus].getProperty(property.id()));
    }
}

QTEST_MAIN(TestStylusPageWidget)

#include "teststyluspage.moc"
