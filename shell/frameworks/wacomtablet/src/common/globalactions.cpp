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

#include "globalactions.h"
#include <KGlobalAccel>
#include <KLocalizedString>
#include <QAction>
#include <QIcon>

using namespace Wacom;

GlobalActions::GlobalActions(bool isConfiguration, QObject *parent)
    : KActionCollection(parent, QLatin1String("wacomtablet"))
{
    setComponentDisplayName(i18n("Wacom Tablet"));
    setConfigGlobal(true);

    QAction *action = addAction(QLatin1String("Toggle touch tool"));
    action->setText(i18nc("@action", "Enable/Disable the Touch Tool"));
    action->setIcon(QIcon::fromTheme(QLatin1String("input-tablet")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_T));
    connect(action, SIGNAL(triggered()), this, SIGNAL(toggleTouchTriggered()));

    action = addAction(QLatin1String("Toggle stylus mode"));
    action->setText(i18nc("@action", "Toggle the Stylus Tool Relative/Absolute"));
    action->setIcon(QIcon::fromTheme(QLatin1String("draw-path")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_S));
    connect(action, SIGNAL(triggered()), this, SIGNAL(toggleStylusTriggered()));

    action = addAction(QLatin1String("Toggle screen map selection"));
    action->setText(i18nc("@action", "Toggle between all screens"));
    action->setIcon(QIcon::fromTheme(QLatin1String("draw-path")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_M));
    connect(action, SIGNAL(triggered()), this, SIGNAL(toggleScreenMapTriggered()));

    action = addAction(QLatin1String("Map to fullscreen"));
    action->setText(i18nc("@action Maps the area of the tablet to all available screen space (space depends on connected monitors)", "Map to fullscreen"));
    action->setIcon(QIcon::fromTheme(QLatin1String("video-display")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_F));
    connect(action, SIGNAL(triggered()), this, SIGNAL(mapToFullScreenTriggered()));

    action = addAction(QLatin1String("Map to screen 1"));
    action->setText(i18nc("@action", "Map to screen 1"));
    action->setIcon(QIcon::fromTheme(QLatin1String("video-display")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_1));
    connect(action, SIGNAL(triggered()), this, SIGNAL(mapToScreen1Triggered()));

    action = addAction(QLatin1String("Map to screen 2"));
    action->setText(i18nc("@action", "Map to screen 2"));
    action->setIcon(QIcon::fromTheme(QLatin1String("video-display")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_2));
    connect(action, SIGNAL(triggered()), this, SIGNAL(mapToScreen2Triggered()));

    action = addAction(QLatin1String("Next Profile"));
    action->setText(i18nc("@action Switch to the next profile in the rotation", "Next profile"));
    action->setIcon(QIcon::fromTheme(QLatin1String("go-next-use")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_N));
    connect(action, SIGNAL(triggered()), this, SIGNAL(nextProfileTriggered()));

    action = addAction(QLatin1String("Previous Profile"));
    action->setText(i18nc("@action Switch to the previous profile in the rotation", "Previous Profile"));
    action->setIcon(QIcon::fromTheme(QLatin1String("go-previous-use")));
    KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_P));
    connect(action, SIGNAL(triggered()), this, SIGNAL(previousProfileTriggered()));

    if (isConfiguration) {
        Q_FOREACH (auto act, actions()) {
            act->setProperty("isConfigurationAction", true);
        }
    }
}

#include "moc_globalactions.cpp"
