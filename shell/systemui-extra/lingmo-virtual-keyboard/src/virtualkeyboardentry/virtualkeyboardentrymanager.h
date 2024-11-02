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

#ifndef VIRTUALKEYBOARDENTRYMANAGER_H
#define VIRTUALKEYBOARDENTRYMANAGER_H

#include <functional>
#include <memory>

#include <QMenu>
#include <QObject>

#include "ipc/fcitxvirtualkeyboardserviceproxy.h"
#include "localsettings/viewlocalsettings.h"
#include "virtualkeyboard/virtualkeyboardmanager.h"
#include "virtualkeyboardentry/floatbuttonmanager.h"
#include "virtualkeyboardentry/virtualkeyboardtrayicon.h"

class VirtualKeyboardEntryManager : public QObject {
    Q_OBJECT

public:
    VirtualKeyboardEntryManager(
        VirtualKeyboardManager &virtualKeyboardManager,
        const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService);
    ~VirtualKeyboardEntryManager() override;

private:
    using ActionTriggeredCallback = std::function<void()>;

private:
    void connectSignals();
    void initTrayIcon(
        const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService);

    void moveValueFromLocalSettings();

    void initFloatButtonContextMenuAndAction();
    void updateFloatButtonContextMenuAction(const QString &icon,
                                            const QString &text,
                                            ActionTriggeredCallback callback);

private:
    VirtualKeyboardManager &virtualKeyboardManager_;

    ViewLocalSettings floatButtonSettings_{"lingmosoft", "lingmo float button"};
    std::unique_ptr<FloatButtonManager> floatButtonManager_ = nullptr;

    std::unique_ptr<VirtualKeyboardTrayIcon> trayIconEntry_ = nullptr;

    std::unique_ptr<QMenu> floatButtonContextMenu_ = nullptr;
    std::unique_ptr<QAction> floatButtonContextMenuAction_ = nullptr;
    ActionTriggeredCallback actionTriggeredCallback_;
};

#endif // VIRTUALKEYBOARDENTRYMANAGER_H
