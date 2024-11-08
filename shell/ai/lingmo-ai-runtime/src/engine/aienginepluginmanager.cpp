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

#include "aienginepluginmanager.h"
#include <filesystem>

namespace ai_engine {

const char *largeModelEngineFactoryInstance = "ai_large_model_engine_factory_instance";
const char *traditionalModelEngineFactoryInstance = "ai_traditional_model_engine_factory_instance";
const char *vectorDatabaseEngineFactoryInstance = "ai_vector_database_engine_factory_instance";

AiEnginePluginManager::AiEnginePluginManager() {
    loadAllEngines();
}

AiEnginePluginManager::~AiEnginePluginManager() {
    unloadAllEngines();
}

std::unique_ptr<lm::nlp::AbstractNlpEngine> AiEnginePluginManager::createNlpEngine(const std::string &engineName) {
    auto iter = lmEngineFactories_.find(engineName);
    if (iter != lmEngineFactories_.end()) {
        return iter->second->createNlpEngine();
    }
    return nullptr;
}

std::unique_ptr<lm::speech::AbstractSpeechEngine> AiEnginePluginManager::createSpeechEngine(const std::string &engineName) {
    auto iter = lmEngineFactories_.find(engineName);
    if (iter != lmEngineFactories_.end()) {
        return iter->second->createSpeechEngine();
    }
    return nullptr;
}

std::unique_ptr<lm::vision::AbstractVisionEngine> AiEnginePluginManager::createVisionEngine(const std::string &engineName) {
    auto iter = lmEngineFactories_.find(engineName);
    if (iter != lmEngineFactories_.end()) {
        return iter->second->createVisionEngine();
    }
    return nullptr;
}

void AiEnginePluginManager::loadAllEngines() {
    loadEngines<lm::AbstractAiEnginePluginFactory>(
        lmEngineFactories_, LARGE_MODEL_PLUGIN_PATH, largeModelEngineFactoryInstance);
    loadEngines<tm::AbstractAiEnginePluginFactory>(
        tmEngineFactories_, TRADITIONAL_MODEL_PLUGIN_PATH, traditionalModelEngineFactoryInstance);
    loadEngines<db_engine::vector_db::AbstractDatabaseEnginePluginFactory>(
        vdEngineFactories_, VECTOR_DATABASE_PLUGIN_PATH, vectorDatabaseEngineFactoryInstance);
}

void AiEnginePluginManager::unloadAllEngines() {
    for (auto &library : engineLibraries_) {
        library.unload();
    }
}

template <typename EngineFactoryType>
EngineFactoryType *AiEnginePluginManager::aiEngineFactory(Library &library, const char *engineFactoryInstance) {
    auto *funcPtr = library.resolve(engineFactoryInstance);
    if (!funcPtr) {
        fprintf(stderr, "Resolve factory instance failed: %s.\n", library.error().c_str());
    }
    auto func = Library::toFunction<EngineFactoryType *()>(funcPtr);
    return func();
}

template<typename EngineFactoryType>
void AiEnginePluginManager::loadEngines(
    std::unordered_map<std::string, EngineFactoryType *> &engineFactoryMap,
    const std::string &pluginPath, const char *engineFactoryInstance) {
    std::filesystem::path path(pluginPath);
    if (!std::filesystem::exists(path)) {
        fprintf(stderr, "Plugin path %s not exists.\n", pluginPath.c_str());
        return;
    }
    for (auto &entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        auto &filePath = entry.path();
        if (filePath.extension() != ".so") {
            continue;
        }

        Library library(filePath.string());
        if (!library.load()) {
            fprintf(stderr, "Load %s failed: %s.\n", filePath.string().c_str(), library.error().c_str());
            continue;
        }
        EngineFactoryType *factory = aiEngineFactory<EngineFactoryType>(library, engineFactoryInstance);
        fprintf(stderr, "Engine name: %s.\n", factory->engineName().c_str());
        engineFactoryMap[factory->engineName()] = factory;
        engineLibraries_.emplace_back(std::move(library));
    }
}

}
