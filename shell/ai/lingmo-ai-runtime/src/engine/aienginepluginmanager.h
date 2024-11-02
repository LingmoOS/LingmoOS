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

#ifndef AIENGINEPLUGINMANAGER_H
#define AIENGINEPLUGINMANAGER_H

#include <lingmo-ai/ai-engine/large-model/aiengine.h>
#include <lingmo-ai/ai-engine/traditional-model/aiengine.h>
#include <lingmo-ai/db-engine/vector-db/dbengine.h>
#include "library.h"

namespace ai_engine {

class AiEnginePluginManager {
public:
    AiEnginePluginManager();
    ~AiEnginePluginManager();

    std::unique_ptr<lm::nlp::AbstractNlpEngine> createNlpEngine(const std::string &engineName);
    std::unique_ptr<lm::speech::AbstractSpeechEngine> createSpeechEngine(const std::string &engineName);
    std::unique_ptr<lm::vision::AbstractVisionEngine> createVisionEngine(const std::string &engineName);

private:
    void loadAllEngines();
    template <typename EngineFactoryType>
    void loadEngines(std::unordered_map<std::string, EngineFactoryType *> &engineFactoryMap,
                     const std::string &pluginPath, const char *engineFactoryInstance);

    void unloadAllEngines();

    template <typename EngineFactoryType>
    EngineFactoryType *aiEngineFactory(Library &library, const char *engineFactoryInstance);

private:
    std::unordered_map<std::string, lm::AbstractAiEnginePluginFactory *> lmEngineFactories_;
    std::unordered_map<std::string, tm::AbstractAiEnginePluginFactory *> tmEngineFactories_;
    std::unordered_map<std::string, db_engine::vector_db::AbstractDatabaseEnginePluginFactory *> vdEngineFactories_;
    std::vector<Library> engineLibraries_;
};

}

#endif // AIENGINEPLUGINMANAGER_H
