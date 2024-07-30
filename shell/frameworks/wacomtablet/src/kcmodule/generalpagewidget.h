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

#ifndef GENERALPAGEWIDGET_H
#define GENERALPAGEWIDGET_H

#include <QWidget>

namespace Ui
{
class GeneralPageWidget;
}

class QDBusInterface;
class KShortcutsEditor;

namespace Wacom
{
class ProfileManagement;
class GlobalActions;

/**
 * This class shows some general information about the detected tablet device.
 *
 * Shows an image and the name plus all detected input devices (pad/stylus/eraser and so on).
 * Mainly used as debug output and to help the user to realize that his tablet
 * was detected correctly.
 */
class GeneralPageWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Default constructor
     *
     * @param parent the parent widget
     */
    explicit GeneralPageWidget(QWidget *parent = nullptr);

    /**
     * default destructor
     */
    ~GeneralPageWidget() override;

    void setTabletId(const QString &tabletId);

    /**
     * Saves all values to the current profile
     */
    void saveToProfile();

public slots:
    /**
     * When called the widget information will be refreshed
     */
    void reloadWidget();

    /**
     * Called whenever the profile is switched or the widget needs to be reinitialized.
     *
     * Updates all values on the widget to the values from the profile.
     */
    void loadFromProfile();

    /**
     * Called whenever a value is changed.
     * Fires the changed() signal afterwards to inform the main widget that unsaved changes are available.
     */
    void profileChanged();

signals:
    /**
     * Used to inform the main widget that unsaved changes in the current profile are available.
     */
    void changed();

private slots:
    void profileUp();
    void profileDown();
    void profileAdd();
    void profileRemove();

private:
    Ui::GeneralPageWidget *ui = nullptr;
    GlobalActions *_actionCollection = nullptr;
    KShortcutsEditor *_shortcutEditor = nullptr;
    QString _tabletId;

}; // CLASS
} // NAMESPACE
#endif // GENERALPAGEWIDGET_H
