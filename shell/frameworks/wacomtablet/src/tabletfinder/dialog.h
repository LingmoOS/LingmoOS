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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include "tabletinformation.h"

namespace Ui
{
class Dialog;
}

class QAbstractButton;

namespace Wacom
{
class HWButtonDialog;

/**
 * @brief Main dialog windows that allows the user to detect and specify the tablet
 */
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

private slots:
    // tablet detection and user selection
    void refreshTabletList();
    void changeTabletSelection(int index);

    // update internal representation on user changes
    void onHasStatusLEDsLeftChanged(bool toggled);
    void onhasStatusLEDsRightChanged(bool toggled);
    void onhasTouchRingChanged(bool toggled);
    void onhasTouchStripLeftChanged(bool toggled);
    void onhasTouchStripRightChanged(bool toggled);
    void onhasWheelChanged(bool toggled);

    void onExpressKeyNumbersChanged(int buttons);
    void buttonBoxClicked(QAbstractButton *button);

    void onPairedIdChanged(int index);
    void onNormalTabletSet(bool enabled);
    void onParentTabletSet(bool enabled);
    void onTouchSensorSet(bool enabled);

    /**
     * @brief Open the dialog that allows the user to detect hardware key mappings
     */
    void onMapButtons();

private:
    bool validIndex();
    void showHWButtonMap();

    Ui::Dialog *m_ui = nullptr;

    struct Tablet {
        int serialID = 0;
        QString company;
        QString name;
        QString layout = QString::fromLatin1("unknown");
        QString model = QString::fromLatin1("unknown");
        QStringList devices;
        bool hasWheel = false;
        bool hasTouchRing = false;
        bool hasTouchStripLeft = false;
        bool hasTouchStripRight = false;
        bool hasStatusLEDsLeft = false;
        bool hasStatusLEDsRight = false;
        int buttonNumber = 0;
        QList<unsigned int> hwMapping;

        QString pairedID;
        bool hasPairedID = false;
        bool isTouchSensor = false;
    };

    void saveTabletInfo(const Tablet &t);

    QList<TabletInformation> m_tabletInformation;
    QList<Tablet> m_tabletList;
    Wacom::HWButtonDialog *m_hwbDialog;
};
}
#endif // DIALOG_H
