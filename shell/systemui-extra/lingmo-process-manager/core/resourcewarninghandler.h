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

#ifndef RESOURCEWARNINGHANDLER_H
#define RESOURCEWARNINGHANDLER_H

#include <string>
#include <memory>

#include <QTimer>

#include "systemnotifiessender.h"

class ConfigManager;
class ResourceWarningHandler
{
public:
    using MemoryWarningCallback = std::function<void(int)>;

    explicit ResourceWarningHandler(std::shared_ptr<ConfigManager> configManager,
                                    std::shared_ptr<SystemNotifiesSender> notifySender);

    void handleResourceWarning(const std::string &resource, int level);
    void setMemoryWarningHanlder(MemoryWarningCallback callback);

private:
    MemoryWarningCallback m_memoryWarningCallback;
    std::shared_ptr<ConfigManager> m_configManager;
    std::shared_ptr<SystemNotifiesSender> m_notifySender;
};

#endif // RESOURCEWARNINGHANDLER_H
