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

#include "kcmwacomtablet.h"
#include "../wacomtablet-version.h"
#include "kcmwacomtabletwidget.h"

#include "aboutdata.h"
#include "dbustabletinterface.h"
#include "profilemanagement.h"

// KDE includes
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

// Qt includes
#include <QVBoxLayout>

using namespace Wacom;

K_PLUGIN_CLASS_WITH_JSON(KCMWacomTablet, "kcm_wacomtablet.json")

KCMWacomTablet::KCMWacomTablet(QObject *parent, const KPluginMetaData &md)
    : KCModule(parent, md)
    , m_changed(false)
{
    initUi();
}

KCMWacomTablet::~KCMWacomTablet()
{
    if (m_layout) {
        delete m_layout;
    }

    if (m_tabletWidget) {
        delete m_tabletWidget;
    }

    // This makes sure all global shortcuts are restored by our daemon.
    // Unfortunately KAction disables all global shortcuts on destruction.
    // There is no way to stop it from doing that so we have to use this
    // workaround and reload the current profile here.
    // BUG restore global shortcuts on exit
    // DBusTabletInterface::instance().setProfile( d->tabletId, , ProfileManagement::instance().profileName() );
}

void KCMWacomTablet::initUi()
{
    setButtons(Apply | Help);

    // setup module ui
    m_tabletWidget = new KCMWacomTabletWidget(widget());
    m_layout = new QVBoxLayout(widget());
    m_layout->setContentsMargins({});
    m_layout->addWidget(m_tabletWidget);

    // connect signals
    connect(m_tabletWidget, &KCMWacomTabletWidget::changed, this, &KCMWacomTablet::setNeedsSave);
}

void KCMWacomTablet::load()
{
    if (m_tabletWidget) {
        m_tabletWidget->reloadProfile();
    }

    setNeedsSave(false);
}

void KCMWacomTablet::save()
{
    if (m_tabletWidget) {
        m_tabletWidget->saveProfile();
    }

    setNeedsSave(false);
}

#include "kcmwacomtablet.moc"

#include "moc_kcmwacomtablet.cpp"
