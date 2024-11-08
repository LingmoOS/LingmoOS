/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors

 * This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <QWidget>
#include <QFont>
#include "src/tools/capturecontext.h"
#include <QLabel>
#include <QPushButton>
class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QComboBox;
class QSpinBox;
class QLabel;
class ImageLabel;

class CaptureLauncher : public QWidget
{
    Q_OBJECT
public:
    explicit CaptureLauncher(QWidget *parent = nullptr);

private slots:
    void startCapture();
    void startDrag();
    void captureTaken(uint id, QPixmap p);
    void captureFailed(uint id);
    void setFont();

private:
    QLabel *m_titleIcon_label, *m_titleName_label;
    QLabel *m_areaLabel, *m_delayLabel;
    QPushButton *m_min_btn, *m_exit_btn;
    QSpinBox *m_delaySpinBox;
    QComboBox *m_captureType;
    QPushButton *m_launchButton;
    QLabel *m_CaptureModeLabel;
    ImageLabel *m_imageLabel;
    QGSettings *m_styleSettings;
    uint m_id;
};
