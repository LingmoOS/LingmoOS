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

#include "prompttemplate.h"
#include <fstream>
namespace config {

std::string readPrompt(const std::string &file) {
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
    return content;
}

PromptTemplate::PromptTemplate(
    const std::string &promptConfigFile, const std::string &promptFilePath) {
    dissectPrompts(promptConfigFile, promptFilePath);
}

PromptTemplate::Prompt PromptTemplate::prompt(PromptId id) const {
    auto iter = prompts_.find(id);
    if (iter != prompts_.end()) {
        return iter->second;
    }
    return { PromptId::Invalid, "", "" };
}

void PromptTemplate::dissectPrompts(
    const std::string &promptFile, const std::string &promptFilePath) {

    fprintf(stderr, "dissectPrompts.%s:%s\n", promptFile.c_str(), promptFilePath.c_str());
    std::ifstream ifs(promptFile);
    if (!ifs.is_open()) {
        return;
    }
    Json::Value root;
    ifs >> root;
    if (!root.isArray()) {
        fprintf(stderr, "Prompt file is not an array of objects.\n");
        return;
    }
    for (const auto &item : root) {
        if (!item.isObject()) {
            fprintf(stderr, "Prompt file is not an array of objects.\n");
            continue;
        }
        
        if (!item.isMember("id") || 
            !item.isMember("prompt-file") ||
            !item.isMember("description")) {
            fprintf(stderr, "Prompt file is not an array of objects with id, prompt and description.\n");
            continue;
        }

        if (!item["id"].isInt() || 
            !item["prompt-file"].isString() ||
            !item["description"].isString()) {
            fprintf(stderr, "Prompt file's key type is incorrect.\n");
            continue;
        }

        PromptId id = static_cast<PromptId>(item["id"].asInt());
        const std::string promptFileName = item["prompt-file"].asString();
        if (promptFileName.empty()) {
            continue;
        }

        const std::string prompt = readPrompt(promptFilePath + "/" + promptFileName);
        prompts_[id] = {
            id, prompt,
            item["description"].asString()
        };
    }
}

} // namespace config
