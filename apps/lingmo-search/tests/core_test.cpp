// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "search.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
using namespace Search;
namespace fs = std::filesystem;
void check(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
int main() {
    std::string path = "/tmp/lingmo-search-test-XXXXXX"; check(::mkdtemp(path.data()), "temp");
    struct Cleanup { std::string path; ~Cleanup() { fs::remove_all(path); } } cleanup{path};
    fs::create_directories(path + "/Documents/.private"); fs::create_directories(path + "/outside");
    std::ofstream(path + "/Documents/设计说明.txt") << "private body not searched";
    std::ofstream(path + "/Documents/Report.txt") << "content-only-marker";
    std::ofstream(path + "/Documents/.private/Report-secret.txt") << "hidden";
    std::ofstream(path + "/outside/Report-outside.txt") << "outside";
    fs::create_directory_symlink(path + "/outside", path + "/Documents/link");
    Request request; request.fileBudget = std::chrono::seconds(2); request.roots = {path + "/Documents"};
    request.applications = {{"editor.desktop", "Editor", "Text editor", "Write documents", "", {"edit", "文档"}},
                            {"files.desktop", "Files", "File manager", "Browse", "", {}}};
    request.settings = {{"display", "显示", "screen resolution"}};
    request.query = "display";
    request.applications.push_back({"other.desktop", "Other app", "", "", "", {"display"}});
    request.settings.push_back({"display-exact", "Display", "screen"});
    auto ranked = search(request); check(ranked.results.front().id == "display-exact", "exact settings title beats application keyword");
    request.query = "EDITOR"; auto result = search(request);
    check(!result.results.empty() && result.results[0].id == "editor.desktop", "case insensitive app search");
    request.query = "文档"; check(search(request).results.front().kind == Kind::Application, "localized keywords");
    request.query = "设计"; result = search(request); check(result.results.size() == 1 && result.results[0].kind == Kind::File, "Chinese filename");
    request.query = "report"; result = search(request); check(result.results.size() == 1, "hidden and symlink paths excluded");
    request.query = "content-only-marker"; check(search(request).results.empty(), "no file content search");
    request.query = "screen"; check(search(request).results.front().kind == Kind::Setting, "setting keywords");
    request.category = Category::Applications; check(search(request).results.empty(), "category isolation");
    request.category = Category::All; request.query = "(12+8)*3";
    result = search(request); check(result.results.front().id == "60" && result.results.front().kind == Kind::Calculation, "calculation");
    request.query = "1/0"; check(search(request).results.empty(), "invalid math excluded");
    request.query = "$(touch /tmp/never)"; check(search(request).results.empty(), "query is never shell code");
    request.query = "report"; request.maxVisited = 0; result = search(request); check(result.fileLimitReached, "scan limit reported");
    request.maxVisited = 100; request.canceled = std::make_shared<std::atomic_bool>(true); check(search(request).results.empty(), "canceled work dropped");
    request.canceled.reset(); request.query = " "; check(search(request).results.empty(), "empty query does not scan");
    request.query = "report"; request.maxResults = 0; check(search(request).results.empty(), "zero result cap");
    check(matchScore("Straße", "STRASSE") == 100, "Unicode casefold");
    std::cout << "Search provider tests passed\n";
}
