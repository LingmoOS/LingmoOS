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

#ifndef SERVICES_SERVICEMANAGER_H
#define SERVICES_SERVICEMANAGER_H

#include <memory>
#include <lingmo-ai/ai-engine/large-model/nlpengine.h>
#include <lingmo-ai/ai-engine/large-model/speechengine.h>
#include <lingmo-ai/ai-engine/large-model/visionengine.h>
#include "services/textprocessor.h"
#include "services/speechprocessor.h"
#include "services/visionprocessor.h"
#include "services/keywordrecognizer.h"
#include "engine/aienginepluginmanager.h"

class ServiceManager {
public:
    explicit ServiceManager(GDBusConnection &connection);

    ~ServiceManager();

private:
    void initProcessors(GDBusConnection &connection);

private:
    ai_engine::AiEnginePluginManager aiEnginePluginManager_;
    std::shared_ptr<TextProcessor> textProcessor_;
    std::shared_ptr<SpeechProcessor> speechProcessor_;
    std::shared_ptr<VisionProcessor> visionProcessor_;
//    std::unique_ptr<KeywordRecognizer> keywordRecognizer_;
};

#endif
