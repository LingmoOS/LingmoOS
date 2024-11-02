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

#ifndef EXPANSIONGEOMETRYMANAGER_H
#define EXPANSIONGEOMETRYMANAGER_H

#include "geometrymanager.h"

class ExpansionGeometryManager : public GeometryManager {

public:
    explicit ExpansionGeometryManager(Scaler &&scaler);
    ~ExpansionGeometryManager() override = default;

private:
    int calculateViewWidth() const override;
    int calculateViewHeight() const override;
    QPoint calculateViewPosition() const override;

private:
    constexpr static float viewHeightRatio_ = 512.0 / 1620.0;
};

#endif // EXPANSIONGEOMETRYMANAGER_H
