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

#ifndef VIRTUALKEYBOARDVIEW_H
#define VIRTUALKEYBOARDVIEW_H

#include <memory>

#include <QObject>
#include <QQuickView>
#include <QString>

#include "animation/animator.h"
#include "geometrymanager/expansiongeometrymanager.h"
#include "geometrymanager/floatgeometrymanager.h"
#include "geometrymanager/geometrymanager.h"
#include "virtualkeyboard/placementmodemanager.h"

class VirtualKeyboardView : public QObject {
    Q_OBJECT

public:
    VirtualKeyboardView(
        QObject &manager, QObject &model,
        std::unique_ptr<PlacementModeManager> placementModeManager,
        std::unique_ptr<ExpansionGeometryManager> expansionGeometryManager,
        std::unique_ptr<FloatGeometryManager> floatGeometryManager);
    ~VirtualKeyboardView() override;

    void initView();

    Q_PROPERTY(bool isFloatMode READ isFloatMode NOTIFY isFloatModeChanged);

    Q_PROPERTY(
        int contentHeight READ getContentHeight NOTIFY contentHeightChanged);
    Q_PROPERTY(
        int contentWidth READ getContentWidth NOTIFY contentWidthChanged);

    void moveBy(int offsetX, int offsetY);
    void endDrag();

    QRect geometry() const;
    void updateGeometry();

    void emitContentGeometrySignals();

    bool isVisible() const;
    void show();
    void hide();
    void flip();

    QWindow *view() { return view_.get(); }

    bool isFloatMode() const { return placementModeManager_->isFloatMode(); }

    void updateExpansionFlippingStartGeometry();
    void setAnimator(std::shared_ptr<Animator> animator);

signals:
    void isFloatModeChanged();

    void contentHeightChanged();
    void contentWidthChanged();

    void updateCandidateArea(const QVariant &candidateTextList,
                             int globalCursorIndex);
    void imDeactivated();

    void raiseAppRequested();
    void fallAppRequested();

public slots:
    void move(int x, int y);

private:
    class State;
    class VisibleState;
    class HidingState;
    class ShowingState;
    class InvisibleState;
    class FlippingState;

private:
    void initState();
    QRect calculateInitialGeometry();
    static int getScreenHeight();
    void connectSignals();
    void destroyView();
    int getContentHeight();
    int getContentWidth();
    void setViewOpacity();

    void updateCurrentState(std::shared_ptr<State> newState);
    void enterVisibleState();
    void enterHidingState();
    void enterShowingState();
    void enterInvisibleState();
    void enterFlippingState();

    GeometryManager &getCurrentGeometryManager() const;

private:
    QObject &manager_;
    QObject &model_;
    std::unique_ptr<QQuickView> view_ = nullptr;
    std::unique_ptr<PlacementModeManager> placementModeManager_ = nullptr;

    std::shared_ptr<Animator> animator_ = nullptr;

    std::shared_ptr<State> currentState_ = nullptr;
    std::shared_ptr<VisibleState> visibleState_ = nullptr;
    std::shared_ptr<HidingState> hidingState_ = nullptr;
    std::shared_ptr<ShowingState> showingState_ = nullptr;
    std::shared_ptr<InvisibleState> invisibleState_ = nullptr;
    std::shared_ptr<FlippingState> flippingState_ = nullptr;

    std::unique_ptr<ExpansionGeometryManager> expansionGeometryManager_ =
        nullptr;
    std::unique_ptr<FloatGeometryManager> floatGeometryManager_ = nullptr;
};

#endif // VIRTUALKEYBOARDVIEW_H
