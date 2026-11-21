#include "repo_manager.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <libintl.h>

#define _(str) gettext(str)

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

bool RepoManager::runRepreproCommand(const std::string& cmd) {
    return std::system(cmd.c_str()) == 0;
} 