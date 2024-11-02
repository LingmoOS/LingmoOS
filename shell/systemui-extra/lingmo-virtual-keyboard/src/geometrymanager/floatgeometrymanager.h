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

#ifndef FLOATGEOMETRYMANAGER_H
#define FLOATGEOMETRYMANAGER_H

#include <memory>

#include "geometrymanager.h"
#include "localsettings/localsettings.h"

class FloatGeometryManager : public GeometryManager {
    Q_OBJECT

public:
    class Strategy;

public:
    FloatGeometryManager(std::unique_ptr<Strategy> strategy,
                         LocalSettings &viewSettings);
    FloatGeometryManager(std::unique_ptr<Strategy> strategy,
                         LocalSettings &viewSettings, Scaler &&scaler);
    ~FloatGeometryManager() override;

public slots:
    void moveBy(int offsetX, int offsetY);
    void endDrag();

private:
    QPoint calculateViewPosition() const override;
    int calculateViewWidth() const override;
    int calculateViewHeight() const override;

    int calculateNormalizedX(int positionX) const;
    int calculateNormalizedY(int positionY) const;
    QPoint calculateNormalizedPosition(const QPoint &position) const;

    QPoint calculatePositionFromRatio(float leftMarginRatio,
                                      float topMarginRatio) const;
    QPoint calculateCurrentPosition() const;
    QPoint calculateNormalizedPositionFromRatio(float leftMarginRatio,
                                                float topMarginRatio) const;

    QSize calculateMarginSize() const;
    QMap<QString, QVariant> getMarginRatioMap() const;
    QMap<QString, QVariant> getDefaultMarginRatioMap() const;
    float calculateLeftMarginRatio(float leftMargin) const;
    float calculateTopMarginRatio(float topMargin) const;
    void updateMarginRatio(const QPoint &targetPosition);
    void saveMarginRatioMap();
    void loadMarginRatioMap();

    void moveView(const QPoint &targetPoint);

private:
    float leftMarginRatio_ = 0.0f;
    float topMarginRatio_ = 0.0f;

    std::unique_ptr<Strategy> strategy_;

    LocalSettings &viewSettings_;

    static const QString floatGeometryGroup;
    static const QString marginRatioMapKey;
    static const QString leftMarginRatioKey;
    static const QString topMarginRatioKey;
};

class FloatGeometryManager::Strategy {
public:
    virtual ~Strategy() = default;

    int getViewWidth() const { return getUnitWidth() * getViewWidthRatio(); }
    int getViewHeight() const { return getUnitHeight() * getViewHeightRatio(); }

    virtual int getDefaultRightMargin() const = 0;
    virtual int getDefaultBottomMargin() const = 0;

protected:
    Strategy() = default;

private:
    virtual int getUnitWidth() const = 0;
    virtual float getViewWidthRatio() const = 0;

    virtual int getUnitHeight() const = 0;
    virtual float getViewHeightRatio() const = 0;
};

#endif // FLOATGEOMETRYMANAGER_H
