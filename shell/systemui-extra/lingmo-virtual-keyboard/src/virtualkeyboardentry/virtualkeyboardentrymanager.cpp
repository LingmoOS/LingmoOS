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

#include "virtualkeyboardentry/virtualkeyboardentrymanager.h"

#include <QAction>
#include <QIcon>

#include "geometrymanager/geometrymanager.h"
#include "virtualkeyboardentry/floatbuttonstrategy.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

static const QString floatButtonGroup = "floatButton";
static const QString floatButtonEnabledKey = "floatButtonEnabled";

VirtualKeyboardEntryManager::VirtualKeyboardEntryManager(
    VirtualKeyboardManager &virtualKeyboardManager,
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService)
    : virtualKeyboardManager_(virtualKeyboardManager),
      floatButtonManager_(new FloatButtonManager(virtualKeyboardManager,
                                                 fcitxVirtualKeyboardService,
                                                 floatButtonSettings_)) {
    moveValueFromLocalSettings();

    initTrayIcon(fcitxVirtualKeyboardService);

    initFloatButtonContextMenuAndAction();

    connectSignals();

    floatButtonManager_->updateFloatButtonEnabled(
        VirtualKeyboardSettings::getInstance().isFloatButtonEnabled());
}

VirtualKeyboardEntryManager::~VirtualKeyboardEntryManager() = default;

void VirtualKeyboardEntryManager::initTrayIcon(
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService) {
    trayIconEntry_.reset(new VirtualKeyboardTrayIcon(
        virtualKeyboardManager_, fcitxVirtualKeyboardService));
}

void VirtualKeyboardEntryManager::initFloatButtonContextMenuAndAction() {
    floatButtonContextMenu_.reset(new QMenu());
    floatButtonContextMenuAction_.reset(new QAction());

    floatButtonContextMenu_->addAction(floatButtonContextMenuAction_.get());

    trayIconEntry_->setContextMenu(floatButtonContextMenu_.get());

    connect(floatButtonContextMenu_.get(), &QMenu::aboutToShow, this,
            [this]() { virtualKeyboardManager_.hide(); });

    connect(floatButtonContextMenuAction_.get(), &QAction::triggered, this,
            [this](bool) {
                if (!actionTriggeredCallback_) {
                    return;
                }

                actionTriggeredCallback_();
            });
}

void VirtualKeyboardEntryManager::connectSignals() {
    connect(floatButtonManager_.get(), &FloatButtonManager::floatButtonEnabled,
            this, [this]() {
                updateFloatButtonContextMenuAction(
                    ":/floatbutton/img/disablefloatbutton.svg",
                    tr("Disable the float button"), []() {
                        VirtualKeyboardSettings::getInstance()
                            .updateFloatButtonAvailability(false);
                    });
            });

    connect(floatButtonManager_.get(), &FloatButtonManager::floatButtonDisabled,
            this, [this]() {
                updateFloatButtonContextMenuAction(
                    ":/floatbutton/img/enablefloatbutton.svg",
                    tr("Enable the float button"), []() {
                        VirtualKeyboardSettings::getInstance()
                            .updateFloatButtonAvailability(true);
                    });
            });

    connect(&virtualKeyboardManager_,
            &VirtualKeyboardManager::virtualKeyboardVisibiltyChanged, this,
            [this](bool visible) {
                if (!visible) {
                    return;
                }

                trayIconEntry_->hideContextMenu();
            });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::requestFloatButtonEnabled, this,
            [this]() {
                virtualKeyboardManager_.hideVirtualKeyboard();
                floatButtonManager_->enableFloatButton();
            });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::requestFloatButtonDisabled, this,
            [this]() { floatButtonManager_->disableFloatButton(); });
}

void VirtualKeyboardEntryManager::updateFloatButtonContextMenuAction(
    const QString &icon, const QString &text,
    ActionTriggeredCallback callback) {
    floatButtonContextMenuAction_->setIcon(QIcon(icon));
    floatButtonContextMenuAction_->setText(text);

    actionTriggeredCallback_ = std::move(callback);
}

void VirtualKeyboardEntryManager::moveValueFromLocalSettings() {
    if (!floatButtonSettings_.contains(floatButtonEnabledKey)) {
        return;
    }

    const bool value =
        floatButtonSettings_.getValue(floatButtonGroup, floatButtonEnabledKey)
            .value<bool>();

    floatButtonSettings_.remove(floatButtonEnabledKey);

    VirtualKeyboardSettings::getInstance().updateFloatButtonAvailability(value);
}
