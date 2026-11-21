// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace Search {
enum class Category { All, Applications, Files, Settings, Calculator };
enum class Kind { Application, File, Setting, Calculation };
struct Result {
    Kind kind;
    std::string id, title, subtitle, icon;
    int score = 0;
};
struct Application { std::string id, name, genericName, comment, icon; std::vector<std::string> keywords; };
struct Setting { std::string id, title, keywords; };
struct Request {
    std::string query;
    Category category = Category::All;
    std::vector<Application> applications;
    std::vector<Setting> settings;
    std::vector<std::string> roots;
    std::shared_ptr<std::atomic_bool> canceled;
    size_t maxVisited = 30000;
    std::chrono::milliseconds fileBudget{300};
    size_t maxResults = 40;
};
struct Response { std::vector<Result> results; bool fileLimitReached = false; size_t visited = 0; };
std::string folded(const std::string& text);
int matchScore(const std::string& text, const std::string& query);
Response search(const Request& request);
std::vector<Setting> settingsCatalog(const std::function<std::string(const char*)>& translate);
}
