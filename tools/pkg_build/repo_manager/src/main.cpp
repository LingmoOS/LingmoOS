#include "repo_manager.h"
#include <iostream>
#include <filesystem>
#include <libintl.h>
#include <locale.h>

#define _(str) gettext(str)

using namespace lingmo;

void printUsage(const char* programName) {
    std::cout << _("Lingmo OS Repository Management Tool") << "\n\n"
              << _("Usage:") << "\n"
              << "  " << programName << " --init <" << _("codename") << "> <" << _("repository directory") << ">\n"
              << "  " << programName << " -c <" << _("codename") << "> <" << _("changes file/directory") << ">\n"
              << "  " << programName << " -deb <" << _("codename") << "> <" << _("deb file/directory") << ">\n"
              << _("Options:") << "\n"
              << "      --init     " << _("Initialize a new repository") << "\n"
              << "  -c, --changes  " << _("Import changes file(s) to repository") << "\n"
              << "  -deb           " << _("Import deb package(s) to repository") << "\n";
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    bindtextdomain("lingmo-repotool", "/usr/share/locale");
    textdomain("lingmo-repotool");

    try {
        if (argc < 2) {
            printUsage(argv[0]);
            return 1;
        }

        std::string arg1 = argv[1];
        
        // 处理仓库初始化
        if (arg1 == "--init") {
            if (argc != 4) {
                std::cerr << _("Error: --init requires codename and repository directory") << "\n";
                return 1;
            }
            std::string codename = argv[2];
            std::filesystem::path repoDir = argv[3];
            return RepoManager::initRepo(repoDir, codename) ? 0 : 1;
        }

        // 处理导入 changes 文件
        if (arg1 == "-c" || arg1 == "--changes") {
            if (argc != 4) {
                std::cerr << _("Error: -c requires codename and changes file/directory") << "\n";
                return 1;
            }
            std::string codename = argv[2];
            std::filesystem::path path = argv[3];
            
            // 自动检测当前目录是否为仓库目录
            std::filesystem::path repoDir = std::filesystem::current_path();
            if (!std::filesystem::exists(repoDir / "conf" / "distributions")) {
                std::cerr << _("Error: Current directory is not a repository") << "\n";
                return 1;
            }

            if (std::filesystem::is_directory(path)) {
                return RepoManager::importChangesDir(repoDir, path, codename) ? 0 : 1;
            } else {
                return RepoManager::importChanges(repoDir, path, codename) ? 0 : 1;
            }
        }

        // 处理导入 deb 包
        if (arg1 == "-deb") {
            if (argc != 4) {
                std::cerr << _("Error: -deb requires codename and deb file/directory") << "\n";
                return 1;
            }
            std::string codename = argv[2];
            std::filesystem::path path = argv[3];
            
            // 自动检测当前目录是否为仓库目录
            std::filesystem::path repoDir = std::filesystem::current_path();
            if (!std::filesystem::exists(repoDir / "conf" / "distributions")) {
                std::cerr << _("Error: Current directory is not a repository") << "\n";
                return 1;
            }

            if (std::filesystem::is_directory(path)) {
                return RepoManager::importDebDir(repoDir, path, codename) ? 0 : 1;
            } else {
                return RepoManager::importDeb(repoDir, path, codename) ? 0 : 1;
            }
        }

        printUsage(argv[0]);
        return 1;

    } catch (const std::exception& e) {
        std::cerr << _("Error") << ": " << e.what() << "\n";
        return 1;
    }
} 