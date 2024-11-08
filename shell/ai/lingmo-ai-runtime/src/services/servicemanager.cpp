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

#include "services/servicemanager.h"
#include <unistd.h>
#include "services/textprocessor.h"

ServiceManager::ServiceManager(GDBusConnection &connection)
{
    initProcessors(connection);
}

ServiceManager::~ServiceManager()
{
    textProcessor_->stopProcess();
    speechProcessor_->stopProcess();
}

void ServiceManager::initProcessors(GDBusConnection &connection)
{
    auto textProcessor = std::make_unique<TextProcessor>(connection, aiEnginePluginManager_);
    textProcessor_ = std::move(textProcessor);
    auto speechProcessor = std::make_unique<SpeechProcessor>(connection, aiEnginePluginManager_);
    speechProcessor_ = std::move(speechProcessor);
    auto visionProcessor = std::make_unique<VisionProcessor>(connection, aiEnginePluginManager_);
    visionProcessor_ = std::move(visionProcessor);
//    keywordRecognizer_ = std::make_unique<KeywordRecognizer>(connection, aiEnginePluginManager_);
}
