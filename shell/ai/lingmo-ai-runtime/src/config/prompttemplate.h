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

#ifndef PROMPTTEMPLATE_H
#define PROMPTTEMPLATE_H

#include <string>
#include <unordered_map> 
#include <jsoncpp/json/json.h>

namespace config {
class PromptTemplate {
public:
    enum class PromptId {
        Invalid = 0,
        TranslateChineseToEnglish = 1,
        TranslateEnglishToChinese = 2,
        TextExpansion = 11,
        TextPolishing = 12,
        TextErrorCorrection = 13,
        EmailReply = 14,
        Summary = 15,
        WorkSummary = 16,
        CodeGeneration = 17,
        MeetingInfoExtraction = 18
    };

    struct Prompt {
        PromptId id;
        std::string prompt;
        std::string description;
    };

    PromptTemplate(const std::string &promptConfigFile, const std::string &promptFilePath);
    Prompt prompt(PromptId id) const;

private:
    void dissectPrompts(const std::string &promptFile, const std::string &promptFilePath);

private:
    std::unordered_map<PromptId, Prompt> prompts_;
};

}

#endif // PROMPTTEMPLATE_H
