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

#ifndef VIRTUALKEYBOARDSTRATEGY_H
#define VIRTUALKEYBOARDSTRATEGY_H

#include "geometrymanager/floatgeometrymanager.h"

#include "screenmanager.h"

class VirtualKeyboardStrategy : public FloatGeometryManager::Strategy {
public:
    VirtualKeyboardStrategy() = default;
    ~VirtualKeyboardStrategy() override = default;

    int getDefaultRightMargin() const override {
        const int screenWidth = ScreenManager::getPrimaryScreenSize().width();

        return (screenWidth - getViewWidth()) / 2;
    }

    int getDefaultBottomMargin() const override {
        const int screenHeight = ScreenManager::getPrimaryScreenSize().height();

        return screenHeight * defaultBottomMarginRatio_;
    }

private:
    int getUnitWidth() const override {
        return ScreenManager::getPrimaryScreenSize().width();
    }

    float getViewWidthRatio() const override { return 1458.0 / 1620.0; }

    int getUnitHeight() const override {
        const auto viewPortSize = ScreenManager::getPrimaryScreenSize();

        return std::max(viewPortSize.width(), viewPortSize.height());
    }

    float getViewHeightRatio() const override { return 548.0 / 1620.0; }

private:
    static constexpr float defaultBottomMarginRatio_ = 0.05f;
};

#endif // VIRTUALKEYBOARDSTRATEGY_H
