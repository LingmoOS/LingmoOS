// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "search.h"
#include "expression.h"
#include <algorithm>
#include <filesystem>
#include <set>
#include <unicode/unistr.h>
namespace Search {
namespace {
namespace fs = std::filesystem;
bool canceled(const Request& request) { return request.canceled && request.canceled->load(); }
bool wants(const Request& request, Category category) { return request.category == Category::All || request.category == category; }
std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}
}
std::string folded(const std::string& text) {
    auto value = icu::UnicodeString::fromUTF8(text); value.foldCase();
    std::string result; value.toUTF8String(result); return result;
}
int matchScore(const std::string& text, const std::string& query) {
    const auto name = folded(text), term = folded(query);
    if (term.empty()) return 0;
    if (name == term) return 100;
    if (name.starts_with(term)) return 90;
    if (name.find(term) != std::string::npos) return 70;
    return 0;
}
Response search(const Request& request) {
    Response response;
    const auto query = trim(request.query);
    if (query.empty() || query.size() > 512 || request.maxResults == 0 || canceled(request)) return response;
    auto append = [&](Result result) {
        response.results.push_back(std::move(result));
        // Keep memory bounded even if every visited filename matches.
        if (response.results.size() > request.maxResults * 2 + 1) {
            std::stable_sort(response.results.begin(), response.results.end(), [](const auto& a, const auto& b) { return a.score > b.score; });
            response.results.resize(request.maxResults);
        }
    };
    if (wants(request, Category::Calculator)) {
        const auto result = SearchMath::evaluate(query);
        const bool expression = query.find_first_of("+-*/^()%!") != std::string::npos || query == "pi" || query == "e";
        if (expression && result.error == SearchMath::Error::None) {
            const auto answer = SearchMath::format(result.value);
            append({Kind::Calculation, answer, answer, query, "accessories-calculator", 120});
        }
    }
    if (wants(request, Category::Applications)) for (const auto& app : request.applications) {
        if (canceled(request)) return {};
        int score = matchScore(app.name, query);
        score = std::max(score, matchScore(app.genericName, query) - 5);
        score = std::max(score, matchScore(app.id, query) - 15);
        score = std::max(score, matchScore(app.comment, query) - 20);
        for (const auto& keyword : app.keywords) score = std::max(score, matchScore(keyword, query) - 5);
        if (score > 0) append({Kind::Application, app.id, app.name, app.genericName.empty() ? app.comment : app.genericName, app.icon, score});
    }
    if (wants(request, Category::Settings)) for (const auto& setting : request.settings) {
        if (canceled(request)) return {};
        const int score = std::max(matchScore(setting.title, query), matchScore(setting.keywords, query) - 10);
        if (score > 0) append({Kind::Setting, setting.id, setting.title, {}, "preferences-system", score});
    }
    if (wants(request, Category::Files)) {
        const auto deadline = std::chrono::steady_clock::now() + request.fileBudget;
        std::set<fs::path> roots, found;
        const std::set<std::string> skip = {"node_modules", "build", "target", "venv", "__pycache__"};
        for (const auto& path : request.roots) {
            std::error_code ec;
            if (fs::is_symlink(fs::symlink_status(path, ec))) continue;
            const auto canonical = fs::canonical(path, ec);
            if (!ec && fs::is_directory(canonical, ec)) roots.insert(canonical);
        }
        std::vector<fs::path> effectiveRoots;
        for (const auto& candidate : roots) {
            bool nested = false;
            for (const auto& parent : effectiveRoots) {
                const auto relative = candidate.lexically_relative(parent);
                if (!relative.empty() && *relative.begin() != "..") { nested = true; break; }
            }
            if (!nested) effectiveRoots.push_back(candidate);
        }
        for (const auto& root : effectiveRoots) {
            if (canceled(request) || response.fileLimitReached) break;
            std::error_code ec;
            fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
            while (it != end) {
                if (canceled(request)) return {};
                if (response.visited >= request.maxVisited || std::chrono::steady_clock::now() >= deadline) {
                    response.fileLimitReached = true; break;
                }
                ++response.visited;
                const auto path = it->path();
                const auto name = path.filename().string();
                const auto status = it->symlink_status(ec);
                const bool hidden = name.empty() || name.front() == '.' || skip.count(name);
                if (hidden || fs::is_symlink(status) || it.depth() >= 12) it.disable_recursion_pending();
                if (!ec && !hidden && !fs::is_symlink(status) && (fs::is_regular_file(status) || fs::is_directory(status))) {
                    const int score = matchScore(name, query);
                    if (score && found.insert(path).second)
                        append({Kind::File, path.string(), name, path.parent_path().string(), fs::is_directory(status) ? "folder" : "text-x-generic", score - 5});
                }
                ec.clear(); it.increment(ec);
                if (ec) break;
            }
        }
    }
    std::stable_sort(response.results.begin(), response.results.end(), [](const Result& a, const Result& b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.kind != b.kind) return a.kind < b.kind;
        return a.title < b.title;
    });
    if (response.results.size() > request.maxResults) response.results.resize(request.maxResults);
    return response;
}
}
