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

#ifndef HWBUTTONDIALOG_H
#define HWBUTTONDIALOG_H

#include <QDialog>

namespace Ui
{
class HWButtonDialog;
}

namespace Wacom
{

/**
 * @brief Dialog that allows the user to press the buttons on the tablet for correct mapping
 *
 * The Tablet buttons 1-x are not directly mapped to the X11 buttons 1-x.
 * Button 4-7 are used for wheel events for example.
 *
 * Also button 1 is not always Button 1 but sometimes 9 and so on
 * This dialog lets the user press the logical tablet buttons and captures the X11 events
 * for the real X11 button connected to it.
 *
 */
class HWButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HWButtonDialog(int maxButtons, QWidget *parent = nullptr);
    ~HWButtonDialog() override;

    QList<unsigned int> buttonMap() const;

protected:
    void mousePressEvent(QMouseEvent *event) final override;

private:
    void hwKey(unsigned int button);
    void nextButton();

    Ui::HWButtonDialog *ui = nullptr;
    int m_maxButtons;
    int m_nextButton;
    QList<unsigned int> m_buttonMap;
};

}

#endif // HWBUTTONDIALOG_H
