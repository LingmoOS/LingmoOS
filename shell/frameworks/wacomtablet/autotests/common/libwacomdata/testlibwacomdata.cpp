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

#include "common/libwacomwrapper.h"
#include "common/tabletdatabase.h"
#include "common/tabletinformation.h"

// #include <QtTest>
#include <QDebug>
#include <QString>

#include <algorithm>

using namespace Wacom;

class TestLibwacomData : public QObject
{
    Q_OBJECT

public:
    void testData();
};

static bool compare(TabletInformation &left, TabletInformation &right, const TabletInfo &info)
{
    if (info == TabletInfo::NumPadButtons) {
        if (left.getInt(info) != right.getInt(info)) {
            qWarning() << "MISMATCH: Property" << info.key() << "mismatch for device" << left.get(TabletInfo::TabletName) << "DB:" << left.getInt(info)
                       << "vs LIB:" << right.getInt(info);
            return false;
        }

        return true;
    }

    if (left.get(info) != right.get(info)) {
        qWarning() << "MISMATCH: Property" << info.key() << "mismatch for device" << left.get(TabletInfo::TabletName) << "DB:" << left.get(info)
                   << "vs LIB:" << right.get(info);
        return false;
    }

    return true;
}

void TestLibwacomData::testData()
{
    using namespace Wacom;

    int missingLocal = 0;
    int missingLibwacom = 0;
    int okay = 0;
    int mismatch = 0;

    int max = 0x0FFF;

    for (int deviceId = 0; deviceId < max; ++deviceId) {
        TabletInformation localInfo(deviceId);
        TabletInformation libwacomInfo(deviceId);

        localInfo.set(TabletInfo::CompanyId, QString::fromLatin1("056A"));
        libwacomInfo.set(TabletInfo::CompanyId, QString::fromLatin1("056A"));

        bool inLocal = false;
        bool inLibWacom = false;

        if (TabletDatabase::instance().lookupTablet(localInfo.get(TabletInfo::TabletId), localInfo.get(TabletInfo::CompanyId), localInfo)) {
            inLocal = true;
        }

        // lookup information in libWacom tablet database
        auto tabletId = localInfo.get(TabletInfo::TabletId).toInt(nullptr, 16);
        auto vendorId = localInfo.get(TabletInfo::CompanyId).toInt(nullptr, 16);
        if (libWacomWrapper::instance().lookupTabletInfo(tabletId, vendorId, libwacomInfo)) {
            inLibWacom = true;
        }

        if (inLocal && inLibWacom) {
            // qDebug() << "Comparing" << deviceId << inLocal << inLibWacom;
            bool identical = true;
            identical &= compare(localInfo, libwacomInfo, TabletInfo::HasLeftTouchStrip);
            identical &= compare(localInfo, libwacomInfo, TabletInfo::HasRightTouchStrip);
            identical &= compare(localInfo, libwacomInfo, TabletInfo::HasTouchRing);
            identical &= compare(localInfo, libwacomInfo, TabletInfo::HasWheel);
            identical &= compare(localInfo, libwacomInfo, TabletInfo::NumPadButtons);
            identical &= compare(localInfo, libwacomInfo, TabletInfo::StatusLEDs);

            // We don't really care about button order here, just compare the indices
            auto localMap = localInfo.getButtonMap().values();
            auto libwacomMap = libwacomInfo.getButtonMap().values();

            std::sort(localMap.begin(), localMap.end());
            std::sort(libwacomMap.begin(), libwacomMap.end());

            bool identicalmapping = localMap.size() == libwacomMap.size() && std::equal(localMap.begin(), localMap.end(), libwacomMap.begin());

            if (!identicalmapping) {
                qWarning() << "MISMATCH: Mapping for device" << localInfo.get(TabletInfo::TabletName) << "differs."
                           << "Local:" << localInfo.getButtonMap() << "Libwacom:" << libwacomInfo.getButtonMap();
            }

            identical &= identicalmapping;

            if (identical) {
                okay++;
                qDebug() << "Device" << localInfo.get(TabletInfo::TabletName) << "is identical";
            } else {
                mismatch++;
            }
        } else if (inLocal) {
            qDebug() << "Device" << localInfo.get(TabletInfo::TabletName) << "is missing from LibWacom";
            missingLibwacom++;
        } else if (inLibWacom) {
            missingLocal++;
        }
    }

    qDebug() << "OK:" << okay << "MISMATCH:" << mismatch << "ONLYLOCAL" << missingLibwacom << "ONLYWACOM" << missingLocal;
}

int main()
{
    TestLibwacomData test;
    test.testData();
}

#include "testlibwacomdata.moc"
