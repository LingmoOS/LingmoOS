/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#include "settings/globalsettings.h"

namespace aisdk
{

GlobalSettings& GlobalSettings::getInstance() {
    static GlobalSettings instance{};

    return instance;
}

GlobalSettings::GlobalSettings() 
    : Settings("org.lingmo.aisdk") {
    childSettingsMap_[CAPABILITY_NLP] = std::make_unique<ChildSettings>("org.lingmo.aisdk.text");
    childSettingsMap_[CAPABILITY_SPEECH] = std::make_unique<ChildSettings>("org.lingmo.aisdk.speech");
    childSettingsMap_[CAPABILITY_VISION] = std::make_unique<ChildSettings>("org.lingmo.aisdk.vision");
}

GlobalSettings::~GlobalSettings() = default;

ChildSettings& GlobalSettings::getChildSettings(Capability capability) {
    return *(childSettingsMap_.at(capability));
}

}
