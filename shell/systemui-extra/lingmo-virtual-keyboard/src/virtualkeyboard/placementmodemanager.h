/*
 * Copyright 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PLACEMENTMODEMANAGER_H
#define PLACEMENTMODEMANAGER_H

#include <QObject>

#include "localsettings/localsettings.h"

class PlacementModeManager : public QObject {
    Q_OBJECT

public:
    explicit PlacementModeManager(LocalSettings &viewSettings);
    ~PlacementModeManager() override;

    bool isFloatMode() const { return isFloatMode_; }

signals:
    void isFloatModeChanged();

public:
    void flipPlacementMode();

private:
    void setPlacementMode(bool isFloatMode);
    void enterExpansionMode();
    void enterFloatMode();
    void savePlacementMode();
    void loadPlacementMode();

private:
    bool isFloatMode_ = true;

    LocalSettings &viewSettings_;

    static const QString placementModeGroup;
    static const QString placementModeKey;
};

#endif // PLACEMENTMODEMANAGER_H
