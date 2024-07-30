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

#ifndef TABLETBACKENDFACTORY_H
#define TABLETBACKENDFACTORY_H

#include "tabletbackendinterface.h"

#include <QMap>
#include <QString>

namespace Wacom
{
class TabletBackendFactory
{
public:
    /**
     * Creates a new instance of the tablet backend. The tablet information
     * parameter already needs to contain some basic tablet information which
     * can be obtained by querying the underlying window system. This should
     * have been done by the event notifier.
     *
     * The returned tablet backend instance need to be deleted once you are
     * done with it.
     *
     * @param info Basic tablet information.
     *
     * @return A new tablet backend instance.
     */
    static TabletBackendInterface *createBackend(const TabletInformation &info);

    /**
     * Helper method for unit testing.
     *
     * Sets the mock object which will be returned by this factory. The mock
     * object will only be returned once as it has to be deleted by the caller.
     * So there is no point in returning the same pointer multiple times.
     *
     * @param mock The mock object returned by this factory, possible null.
     */
    static void setTabletBackendMock(TabletBackendInterface *mock);

    /**
     * Sets unit testing mode.
     *
     * In unit testing mode this factory will always return the tablet backend
     * mock or NULL.
     */
    static void setUnitTest(bool isUnitTest);

protected:
    /**
     * Creates a tablet backend instance.
     *
     * @param info      The tablet information.
     *
     * @return A new instance of a tablet backend.
     */
    TabletBackendInterface *createInstance(const Wacom::TabletInformation &info);

private:
    //! Private default constructor as this is a static class.
    TabletBackendFactory();

    //! Copy constructor which does nothing.
    TabletBackendFactory(const TabletBackendFactory &factory) = delete;

    //! Copy operator which does nothing.
    TabletBackendFactory &operator=(const TabletBackendFactory &factory) = delete;

    //! The mock object returned by this factory if it is set.
    static TabletBackendInterface *m_tabletBackendMock;
    static bool m_isUnitTest;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
