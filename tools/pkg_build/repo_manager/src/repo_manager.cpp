#include "repo_manager.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <libintl.h>

#define _(str) gettext(str)

namespace lingmo {

bool RepoManager::s_repreproChecked = false;

bool RepoManager::checkReprepro() {
    if (s_repreproChecked) return true;
    
    if (std::system("which reprepro > /dev/null 2>&1") != 0) {
        std::cerr << _("Error: reprepro not found. Please install with:") << "\n"
                  << "sudo apt install reprepro\n";
        return false;
    }
    
    s_repreproChecked = true;
    return true;
}

bool RepoManager::runRepreproCommand(const std::string& cmd) {
    return std::system(cmd.c_str()) == 0;
}

bool RepoManager::createRepoConfig(const std::filesystem::path& repoDir, 
                                 const std::string& codename) {
    auto confDir = repoDir / "conf";
    std::filesystem::create_directories(confDir);

    std::ofstream dist(confDir / "distributions");
    if (!dist.is_open()) {
        std::cerr << _("Error: Unable to create distributions file") << "\n";
        return false;
    }

    dist << "Origin: Lingmo OS\n"
         << "Label: Lingmo\n"
         << "Codename: " << codename << "\n"
         << "Architectures: i386 amd64 arm64 source\n"
         << "Components: main\n"
         << "Description: Lingmo OS " << codename << " Repository\n"
         << "SignWith: yes\n";

    return true;
}

bool RepoManager::initRepo(const std::filesystem::path& repoDir, 
                         const std::string& codename) {
    if (!checkReprepro()) return false;

    std::filesystem::create_directories(repoDir);

    if (!createRepoConfig(repoDir, codename)) {
        std::cerr << _("Error: Failed to create repository configuration") << "\n";
        return false;
    }

    std::cout << _("Repository initialized successfully at") << ": " << repoDir << "\n";
    return true;
}

bool RepoManager::importChanges(const std::filesystem::path& repoDir,
                              const std::filesystem::path& changesFile,
                              const std::string& codename) {
    if (!checkReprepro()) return false;

    if (!std::filesystem::exists(changesFile)) {
        std::cerr << _("Error: Changes file not found") << ": " << changesFile << "\n";
        return false;
    }

    std::string cmd = "cd " + repoDir.string() + " && "
                    + "reprepro -V --ignore=wrongdistribution include "
                    + codename + " "
                    + changesFile.string();

    return runRepreproCommand(cmd);
}

bool RepoManager::importChangesDir(const std::filesystem::path& repoDir,
                                 const std::filesystem::path& directory,
                                 const std::string& codename) {
    if (!checkReprepro()) return false;

    if (!std::filesystem::exists(directory)) {
        std::cerr << _("Error: Directory not found") << ": " << directory << "\n";
        return false;
    }

    bool success = true;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".changes") {
            std::cout << _("Importing") << " " << entry.path().filename() << "...\n";
            if (!importChanges(repoDir, entry.path(), codename)) {
                std::cerr << _("Failed to import") << " " << entry.path() << "\n";
                success = false;
            }
        }
    }

    return success;
}

bool RepoManager::importDeb(const std::filesystem::path& repoDir,
                          const std::filesystem::path& debFile,
                          const std::string& codename,
                          const std::string& component) {
    if (!checkReprepro()) return false;

    // 检查是否存在对应的源码包
    auto debPath = debFile.string();
    auto dscPath = debPath.substr(0, debPath.rfind("_")) + ".dsc";
    
    bool success = true;

    // 如果存在源码包，先导入源码包
    if (std::filesystem::exists(dscPath)) {
        std::string srcCmd = "cd " + repoDir.string() + " && "
                         + "reprepro -V includedsc "
                         + codename + " "
                         + dscPath;
        
        if (!runRepreproCommand(srcCmd)) {
            std::cerr << _("Warning: Failed to import source package") << "\n";
            success = false;
        }
    }

    // 导入二进制包
    std::string cmd = "cd " + repoDir.string() + " && "
                    + "reprepro -V includedeb "
                    + codename + " "
                    + debFile.string();

    return runRepreproCommand(cmd) && success;
}

bool RepoManager::importDebDir(const std::filesystem::path& repoDir,
                             const std::filesystem::path& directory,
                             const std::string& codename,
                             const std::string& component) {
    if (!checkReprepro()) return false;

    bool success = true;

    // 先导入所有源码包
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".dsc") {
            std::cout << _("Importing source") << " " << entry.path().filename() << "...\n";
            std::string srcCmd = "cd " + repoDir.string() + " && "
                             + "reprepro -V includedsc "
                             + codename + " "
                             + entry.path().string();
            
            if (!runRepreproCommand(srcCmd)) {
                std::cerr << _("Failed to import source") << " " << entry.path() << "\n";
                success = false;
            }
        }
    }

    // 再导入所有二进制包
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".deb") {
            std::cout << _("Importing binary") << " " << entry.path().filename() << "...\n";
            std::string cmd = "cd " + repoDir.string() + " && "
                          + "reprepro -V includedeb "
                          + codename + " "
                          + entry.path().string();
            
            if (!runRepreproCommand(cmd)) {
                std::cerr << _("Failed to import binary") << " " << entry.path() << "\n";
                success = false;
            }
        }
    }

    return success;
}

} // namespace lingmo 