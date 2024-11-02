/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SYSTEMNOTIFIESSENDER_H
#define SYSTEMNOTIFIESSENDER_H

#include <memory>

#include <QTimer>

#include "notifydbusinterface.h"
#include "configmanager.h"

class SystemNotifiesSender
{
public:
    SystemNotifiesSender(std::shared_ptr<ConfigManager> configManager);

    void sendResourceWarningNotify();

private:
    void initConnections();
    void handleResourceWarningNotifyClose(uint notifyId, uint reason);
    void handleResourceWarningActionInvoked(uint notifyId, const std::string &actionKey);

    std::shared_ptr<ConfigManager> m_configManager;
    std::unique_ptr<NotifyDBusInterface> m_notifyDBusInterface;
    std::unique_ptr<QTimer> m_notifyTimer;
    uint m_resourceWarningNotifyId;
};

#endif // SYSTEMNOTIFIESSENDER_H
