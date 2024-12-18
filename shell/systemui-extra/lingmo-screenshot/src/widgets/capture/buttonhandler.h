/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*             2020 KylinSoft Co., Ltd.
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

#include "capturebutton.h"
#include <QVector>
#include <QObject>
#include <QRegion>

class CaptureButton;
class QRect;
class QPoint;

class ButtonHandler : public QObject {
    Q_OBJECT
public:
    ButtonHandler(const QVector<CaptureButton*>&, QObject *parent = nullptr);
    explicit ButtonHandler(QObject *parent = nullptr);

    void hideSectionUnderMouse(const QPoint &p);

    bool isVisible() const;
    bool buttonsAreInside() const;
    size_t size() const;

    void setButtons(const QVector<CaptureButton*>);
    bool contains(const QPoint &p) const;
    void updateScreenRegions(const QVector<QRect> &rects);
    void updateScreenRegions(const QRect &rect);

    void clearButtons();

    int move_FontSize_Color_Chose_Window(int y);
    QPoint move_Save_Location_Window(int x,int y);
    QPoint move_Font_Options_Window(int x,int y);

    int FontSize_Color_Chose_Window_Y;
    QPoint Save_Location_Window_Pos;
    QPoint Font_Options_Window_Pos;
public slots:
    void updatePosition(const QRect &selection);
    void hide();
    void show();

private:
    QVector<QPoint> horizontalPoints(const QPoint &center, const int elements,
                               const bool leftToRight) const;
    QVector<QPoint> verticalPoints(const QPoint &center, const int elements,
                               const bool upToDown) const;

    QRect intersectWithAreas(const QRect &rect);

    QVector<CaptureButton*> m_vectorButtons;

    QRegion m_screenRegions;

    QRect m_selection;
    QVector<QRect> allrect;
    int m_separator;
    int m_buttonExtendedSize;
    int m_buttonBaseSize;

    bool m_buttonsAreInside;
    bool m_blockedRight;
    bool m_blockedLeft;
    bool m_blockedBotton;
    bool m_blockedTop;
    bool m_oneHorizontalBlocked;
    bool m_horizontalyBlocked;
    bool m_allSidesBlocked;
    bool m_isTabletMode; // 平板模式

    // aux methods
    void init();
    void resetRegionTrack();
    void updateBlockedSides();
    void expandSelection();
    void positionButtonsInside(int index);
    void ensureSelectionMinimunSize();
    void moveButtonsToPoints(const QVector<QPoint> &points, int &index);
    void adjustHorizontalCenter(QPoint &center);
};
