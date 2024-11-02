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

#ifndef FLOATBUTTONSTRATEGY_H
#define FLOATBUTTONSTRATEGY_H

#include "geometrymanager/floatgeometrymanager.h"
#include "screenmanager.h"

class FloatButtonStrategy : public FloatGeometryManager::Strategy {
public:
    FloatButtonStrategy() = default;
    ~FloatButtonStrategy() override = default;

    int getDefaultRightMargin() const override {
        return getUnitWidth() * defaultRightMarginRatio_;
    }

    int getDefaultBottomMargin() const override {
        return getUnitHeight() * defaultBottomMarginRatio_;
    }

private:
    int getUnitWidth() const override {
        const auto viewPortSize = ScreenManager::getPrimaryScreenSize();

        return std::max(viewPortSize.width(), viewPortSize.height());
    }

    int getUnitHeight() const override {
        const auto viewPortSize = ScreenManager::getPrimaryScreenSize();

        return std::max(viewPortSize.width(), viewPortSize.height());
    }

    float getViewWidthRatio() const override { return 56.0 / 1620.0; }

    float getViewHeightRatio() const override { return 56.0 / 1620.0; }

private:
    static constexpr float defaultBottomMarginRatio_ = 0.03f;
    static constexpr float defaultRightMarginRatio_ = 0.02f;
};

#endif // FLOATBUTTONSTRATEGY_H
