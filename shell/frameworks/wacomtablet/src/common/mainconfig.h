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

#ifndef MAINCONFIG_H
#define MAINCONFIG_H

#include <QString>

namespace Wacom
{

class MainConfigPrivate;

class MainConfig
{
public:
    MainConfig();
    explicit MainConfig(const QString &fileName);
    ~MainConfig();

    QString getLastProfile(const QString &deviceName);

    void open(const QString &fileName);

    void setLastProfile(const QString &deviceName, const QString &profile);

private:
    Q_DECLARE_PRIVATE(MainConfig)
    MainConfigPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
