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

#ifndef FLOATBUTTONMANAGER_H
#define FLOATBUTTONMANAGER_H

#include <memory>

#include <QDateTime>
#include <QObject>
#include <QPushButton>

#include "localsettings/localsettings.h"
#include "virtualkeyboard/virtualkeyboardmanager.h"
#include "virtualkeyboardentry/fcitxvirtualkeyboardservice.h"
#include "virtualkeyboardentry/floatbutton.h"

class FloatButtonManager : public QObject {
    Q_OBJECT

public:
    FloatButtonManager(
        const VirtualKeyboardManager &virtualKeyboardManager,
        const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService,
        LocalSettings &floatButtonSettings);
    ~FloatButtonManager() override = default;

    void updateFloatButtonEnabled(bool enabled);

    void enableFloatButton();
    void disableFloatButton();

signals:
    void floatButtonEnabled();
    void floatButtonDisabled();

private slots:
    void initFloatButton();
    void destroyFloatButton();

    void onScreenResolutionChanged();

private:
    void initGeometryManager();

    void initInternalSignalConnections();
    void initScreenSignalConnections();

    void createFloatButton();
    void connectFloatButtonSignals();

    void showFloatButton();
    void hideFloatButton();

    void setFloatButtonEnabled(bool enabled);

private:
    bool floatButtonEnabled_ = false;

    const VirtualKeyboardManager &virtualKeyboardManager_;
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService_;

    LocalSettings &floatButtonSettings_;

    std::unique_ptr<FloatButton> floatButton_ = nullptr;

    std::unique_ptr<FloatGeometryManager> geometryManager_ = nullptr;
};

#endif // FLOATBUTTONMANAGER_H
