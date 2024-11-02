/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include "windowmanager/windowmanager.h"

using namespace kdk;
class QSpinBox;
class QComboBox;
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void initUI();
    void initConnection();

private:
    QLabel* m_label;
    QWidget* m_subWidget;
    QPushButton* m_btnMinimize;
    QPushButton* m_btnMaximum;
    QPushButton* m_btnActivate;
    QPushButton* m_btnStayOnTop;
    QPushButton* m_btnClose;
    QPushButton* m_btnShowDesktop;
    QPushButton* m_btnChangeIcon;
    QPushButton* m_btnChangeTitle;
    QPushButton* m_btnShowGroup;
    QPushButton* m_shwWdgbtn;
    QPushButton* m_shwWindowTitle;
    QPushButton* m_shwWindowIcon;
    QPushButton* m_btnprintList;
    QPushButton* m_btnSetGeometry;
    QPushButton* m_btnGetType;
    QComboBox* m_setTypeBox;
    kdk::WindowManager *m_manager;
    QSpinBox *m_posX;
    QSpinBox *m_posY;
    QSpinBox *m_width;
    QSpinBox *m_height;
    QTimer* m_timer;
    WindowId m_lastestWindowId;
    QPushButton* m_skipTaskbarBtn;
    QPushButton* m_skipSwitcherBtn;
    QPushButton* m_showOnAllDesktop;
    QPushButton* m_isDesktopShowing;
    QPushButton* m_isOnCurrentDesktop;
    QPushButton* m_btnPanelWidget;
    QCheckBox* m_chbPanel;
    bool m_isPanelShown;
    QWidget* m_panelWidget;
    QPushButton* m_demandBtn;
    QPushButton* m_fullscreenBtn;
    QPushButton* m_processName;
    QPushButton* m_getGeometryBtn;
    QPushButton* m_setGrabKeyboardBtn;
    QPushButton* m_setSwitcherLayer;
    QPushButton* m_setHighlightBtn;
    QPushButton* m_unsetHighlightBtn;
    QPushButton* m_openUnderCursorBtn;
    QPushButton* m_openUnderCursorWithOffsetBtn;
    QSpinBox *m_xOffset;
    QSpinBox *m_yOffset;
    QPushButton* m_setIconNameBtn;
    QPushButton* m_unsetIconNameBtn;
    QPushButton* m_getCurrentOutputBtn;
    QPushButton* m_getOutputsInfoBtn;
    QPushButton* m_getWindowIdByTitleBtn;
    QLineEdit* m_getWindowIdByTitleEdit;
    QPushButton* m_getWindowIdByPidBtn;
    QLineEdit* m_getWindowIdByPidEdit;
};
#endif // WIDGET_H
