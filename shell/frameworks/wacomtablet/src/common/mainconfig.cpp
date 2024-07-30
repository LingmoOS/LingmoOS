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

#include "mainconfig.h"

#include <KConfigGroup>
#include <KSharedConfig>

using namespace Wacom;

namespace Wacom
{
class MainConfigPrivate
{
public:
    KSharedConfig::Ptr config;
    KConfigGroup general;
}; // CLASS
} // NAMESPACE

MainConfig::MainConfig()
    : d_ptr(new MainConfigPrivate)
{
    open(QLatin1String("wacomtablet-kderc"));
}

MainConfig::MainConfig(const QString &fileName)
    : d_ptr(new MainConfigPrivate)
{
    open(fileName);
}

MainConfig::~MainConfig()
{
    Q_D(MainConfig);
    if (d->config) {
        d->config->sync();
    }

    delete this->d_ptr;
}

void MainConfig::open(const QString &fileName)
{
    Q_D(MainConfig);
    d->config = KSharedConfig::openConfig(fileName);
    d->general = KConfigGroup(d->config, QStringLiteral("LastProfile"));
}

QString MainConfig::getLastProfile(const QString &deviceName)
{
    Q_D(MainConfig);
    QString profile;

    if (d->config) {
        d->config->reparseConfiguration();
        profile = d->general.readEntry(deviceName);
    }

    return profile;
}

void MainConfig::setLastProfile(const QString &deviceName, const QString &profile)
{
    Q_D(MainConfig);
    if (d->config) {
        d->config->reparseConfiguration();
        d->general.writeEntry(deviceName, profile);
        d->general.sync();
    }
}
