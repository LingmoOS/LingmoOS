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

#ifndef TABLETDEPENDENTTEST_H
#define TABLETDEPENDENTTEST_H

#include "common/tabletinformation.h"

#include <QtTest>

namespace Wacom
{
/**
 * The base class for tablet dependent test suites.
 *
 * This class makes sure a tablet is actually connected to the system.
 */
class TabletDependentTest : public QObject
{
    Q_OBJECT

public:
    TabletDependentTest(QObject *parent = NULL);

    void findTablet();

    //! Returns tablet information about the tablet which was detected.
    const TabletInformation &getTabletInformation() const;

    //! Returns if a tablet is available.
    bool isTabletAvailable() const;

    //! Prints the tablet information to stdout.
    void printTabletInformation(const TabletInformation &info) const;

protected:
    virtual void initTestCaseDependent() = 0;

private slots:

    void initTestCase();

private:
    TabletInformation m_tabletInformation;
    bool m_isTabletAvailable;

}; // CLASS
} // NAMEPSPACE
#endif // HEADER PROTECTION
