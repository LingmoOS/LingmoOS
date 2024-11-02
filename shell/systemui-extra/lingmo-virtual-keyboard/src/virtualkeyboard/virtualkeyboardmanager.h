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

#ifndef VIRTUALKEYBOARDMANAGER_H
#define VIRTUALKEYBOARDMANAGER_H

#include <functional>
#include <memory>

#include <QObject>

#include "animation/animationfactory.h"
#include "appinputareamanager.h"
#include "geometrymanager/expansiongeometrymanager.h"
#include "geometrymanager/floatgeometrymanager.h"
#include "localsettings/viewlocalsettings.h"
#include "placementmodemanager.h"
#include "virtualkeyboardmodel.h"
#include "virtualkeyboardview.h"

class VirtualKeyboardManager : public QObject {
    Q_OBJECT

public:
    using HideVirtualKeyboardCallback = std::function<void()>;

public:
    explicit VirtualKeyboardManager(
        HideVirtualKeyboardCallback hideVirtualKeyboardCallback);
    ~VirtualKeyboardManager();

    void showVirtualKeyboard();

    Q_INVOKABLE void hide();
    Q_INVOKABLE void flipPlacementMode();
    Q_INVOKABLE void moveBy(int offsetX, int offsetY);
    Q_INVOKABLE void endDrag();
    void visibiltyChanged();
    bool isVirtualKeyboardVisible() const;
    void updatePreeditCaret(int index);
    void updatePreeditArea(const QString &preeditText);
    void updateCandidateArea(const QStringList &candidateTextList, bool hasPrev,
                             bool hasNext, int pageIndex,
                             int globalCursorIndex);
    void notifyIMActivated(const QString &uniqueName);
    void notifyIMDeactivated(const QString &uniqueName);
    void notifyIMListChanged();

signals:
    void virtualKeyboardVisibiltyChanged(bool isShow);

public slots:
    void processResolutionChangedEvent();

    void hideVirtualKeyboard();

private:
    void initAppInputAreaManager();
    std::unique_ptr<PlacementModeManager> createPlacementModeManager();
    static Scaler createExpansionModeScaler();
    static Scaler createFloatModeScaler();
    static std::unique_ptr<ExpansionGeometryManager>
    createExpansionGeometryManager();
    std::unique_ptr<FloatGeometryManager> createFloatGeometryManger();

    void initVirtualKeyboardModel();

    void initScreenSignalConnections();

    void initVirtualKeyboardView();
    void connectVirtualKeyboardModelSignals();
    void connectVirtualKeyboardViewSignals();
    void connectVirtualKeyboardSettingsSignals();

    void raiseInputAreaIfNecessary();

    std::unique_ptr<AnimationFactory> createAnimationFactory();
    std::unique_ptr<Animator> createEnabledAnimator();
    std::unique_ptr<Animator> createDisabledAnimator();
    std::unique_ptr<Animator> createAnimator();

    std::unique_ptr<AppInputAreaManager> appInputAreaManager_ = nullptr;
    std::unique_ptr<VirtualKeyboardModel> model_ = nullptr;
    std::unique_ptr<VirtualKeyboardView> view_ = nullptr;

    HideVirtualKeyboardCallback hideVirtualKeyboardCallback_;

    ViewLocalSettings viewSettings_{"lingmosoft", "lingmo virtual keyboard"};
};

#endif // VIRTUALKEYBOARDMANAGER_H
