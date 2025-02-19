#include "lingmo_pkgbuild.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <libintl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define _(str) gettext(str)

// 添加静态成员初始化
std::filesystem::path LingmoPkgBuilder::s_buildDir = ".build_deb_lingmo";
std::filesystem::path LingmoPkgBuilder::s_outputDir = "pkg_out";
int LingmoPkgBuilder::s_threadCount = 1;  // 默认单线程
bool LingmoPkgBuilder::s_signBuild = true;        // 默认进行签名
std::string LingmoPkgBuilder::s_signKey = "";     // 默认使用默认密钥

LingmoPkgBuilder::LingmoPkgBuilder(const std::filesystem::path& sourceDir, PackageType type)
    : m_packageType(type) {
    // 先从 changelog 获取正确的包名
    std::string correctName;
    {
        std::ifstream changelog(sourceDir / "debian/changelog");
        if (!changelog.is_open()) {
            throw std::runtime_error(_("Unable to open changelog file"));
        }

        std::string line;
        if (std::getline(changelog, line)) {
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos) {
                correctName = line.substr(0, spacePos);
            }
        }
    }

    if (correctName.empty()) {
        throw std::runtime_error(_("Unable to get package name from changelog"));
    }

    m_tempDir = s_buildDir / correctName;
    std::filesystem::create_directories(m_tempDir);

    if (sourceDir.parent_path() != s_buildDir) {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(sourceDir)) {
                const auto& path = entry.path();
                auto destPath = m_tempDir / entry.path().filename();
                if (entry.is_directory()) {
                    std::filesystem::copy(path, destPath,
                        std::filesystem::copy_options::recursive |
                        std::filesystem::copy_options::overwrite_existing);
                } else {
                    std::filesystem::copy(path, destPath,
                        std::filesystem::copy_options::overwrite_existing);
                }
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string(_("Failed to copy source files")) + ": " + e.what());
        }
    }

    if (!parseChangelogFile(m_tempDir / "debian/changelog")) {
        std::cerr << _("Warning: Unable to get version from changelog") << "\n";
    }
    
    if (!parseControlFile(m_tempDir / "debian/control")) {
        throw std::runtime_error(_("Failed to parse control file"));
    }
    
    auto formatFile = m_tempDir / "debian/source/format";
    if (std::filesystem::exists(formatFile)) {
        std::ifstream format(formatFile);
        std::string formatStr;
        std::getline(format, formatStr);
        if (formatStr == "3.0 (quilt)") {
            m_packageType = PackageType::Quilt;
        } else if (formatStr == "3.0 (native)") {
            m_packageType = PackageType::Native;
        }
    }

    m_packageName = correctName;
}

void LingmoPkgBuilder::setMaintainer(const std::string& maintainer) {
    m_maintainer = maintainer;
}

void LingmoPkgBuilder::setDescription(const std::string& description) {
    m_description = description;
}

bool LingmoPkgBuilder::createControlFile() {
    auto controlDir = m_tempDir / "DEBIAN";
    std::filesystem::create_directories(controlDir);
    
    std::ofstream control(controlDir / "control");
    if (!control.is_open()) return false;

    control << "Package: " << m_packageName << "\n"
            << "Version: " << m_version << "\n"
            << "Architecture: " << m_architecture << "\n"
            << "Maintainer: " << m_maintainer << "\n"
            << "Description: " << m_description << "\n";
    
    return true;
}

void LingmoPkgBuilder::addFile(const std::string& sourcePath, const std::string& destPath) {
    auto targetPath = m_tempDir / destPath;
    std::filesystem::create_directories(targetPath.parent_path());
    std::filesystem::copy_file(sourcePath, targetPath);
}

bool LingmoPkgBuilder::createOrigTarball() const {
    if (isNativePackage()) {
        return true;  // 原生包不需要 orig tarball
    }

    try {
        // 获取上游版本号（移除 debian 修订版本）
        std::string upstreamVersion = m_version;
        size_t dashPos = m_version.find('-');
        if (dashPos != std::string::npos) {
            upstreamVersion = m_version.substr(0, dashPos);
        }
        
        // 创建一个临时的源码目录
        auto tempSourceDir = m_tempDir / m_packageName;
        std::filesystem::create_directories(tempSourceDir);

        // 复制源文件到临时目录
        for (const auto& entry : std::filesystem::directory_iterator(m_tempDir)) {
            const auto& filename = entry.path().filename();
            if (filename != "debian" && filename != m_packageName) {
                if (entry.is_directory()) {
                    std::filesystem::copy(entry.path(), tempSourceDir,
                        std::filesystem::copy_options::recursive);
                } else {
                    std::filesystem::copy(entry.path(), tempSourceDir / filename);
                }
            }
        }

        // 创建 orig tarball，使用正确的目录名
        std::string tarCmd = "cd " + m_tempDir.string() + " && "
                          + "tar -Jcf " + m_packageName + "_" + upstreamVersion + ".orig.tar.xz "
                          + m_packageName;
        
        bool result = std::system(tarCmd.c_str()) == 0;
        
        // 清理临时源码目录
        std::filesystem::remove_all(tempSourceDir);
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create orig tarball: " << e.what() << "\n";
        return false;
    }
}

bool LingmoPkgBuilder::build(const std::filesystem::path& sourceDir) {
    try {
        std::string cpCmd = "cp -r " + sourceDir.string() + "/* " + m_tempDir.string();
        if (!runCommand(cpCmd)) {
            std::cerr << _("Failed to copy source files") << "\n";
            return false;
        }

        if (!isNativePackage()) {
            std::string upstreamVersion = m_version;
            size_t dashPos = m_version.find('-');
            if (dashPos != std::string::npos) {
                upstreamVersion = m_version.substr(0, dashPos);
            }

            std::string tarCmd = "cd " + m_tempDir.parent_path().string() + " && "
                              + "tar --exclude=debian -Jcf " + m_packageName + "_" + upstreamVersion + ".orig.tar.xz "
                              + "-C " + m_packageName + " .";

            if (!runCommand(tarCmd)) {
                std::cerr << _("Failed to create orig tarball") << "\n";
                return false;
            }
        }

        std::string buildCmd = "cd " + m_tempDir.string() + " && dpkg-buildpackage";
        
        if (s_threadCount > 1) {
            buildCmd += " -j" + std::to_string(s_threadCount);
        }
        
        if (!s_signBuild) {
            buildCmd += " -us -uc --no-sign";
        } else if (!s_signKey.empty()) {
            buildCmd += " -k" + s_signKey;
        }
        
        if (!isNativePackage()) {
            buildCmd += " -sa";
        }

        if (!runCommand(buildCmd)) {
            std::cerr << _("Build command failed") << "\n";
            return false;
        }

        if (!copyArtifacts(m_packageName)) {
            std::cerr << _("Failed to copy artifacts") << "\n";
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << _("Build failed") << ": " << e.what() << "\n";
        return false;
    }
}

bool LingmoPkgBuilder::parseChangelogFile(const std::filesystem::path& changelogFile) {
    std::cout << _("Parsing changelog file") << ": " << changelogFile << "\n";
    
    std::ifstream file(changelogFile);
    if (!file.is_open()) {
        std::cerr << _("Unable to open changelog file") << "\n";
        return false;
    }

    std::string line;
    if (std::getline(file, line)) {
        // changelog 的第一行格式: package (version) distribution; urgency=level
        size_t start = line.find('(');
        if (start != std::string::npos) {
            size_t end = line.find(')', start);
            if (end != std::string::npos) {
                m_version = line.substr(start + 1, end - start - 1);
                std::cout << "Found version from changelog: " << m_version << "\n";
                return true;
            }
        }
    }

    return false;
}

bool LingmoPkgBuilder::parseControlFile(const std::filesystem::path& controlFile) {
    std::cout << _("Parsing control file") << ": " << controlFile << "\n";
    
    std::ifstream file(controlFile);
    if (!file.is_open()) {
        std::cerr << _("Unable to open control file") << "\n";
        return false;
    }

    // 重置除版本号外的所有字段
    m_packageName.clear();
    m_architecture.clear();
    m_maintainer.clear();
    m_description.clear();
    // 不重置 m_version，保留从 changelog 读取的版本号

    std::string line;
    bool foundSource = false;
    while (std::getline(file, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') continue;
        
        // 如果遇到新段落（以空格开头的行是上一行的延续）
        if (!line.empty() && !std::isspace(line[0])) {
            if (line.compare(0, 9, "Package: ") == 0) {
                m_packageName = line.substr(9);
                std::cout << "Found package name: " << m_packageName << "\n";
            }
            else if (line.compare(0, 9, "Version: ") == 0) {
                // 只有在没有从 changelog 读取到版本号时才使用 control 文件中的版本
                if (m_version.empty()) {
                    m_version = line.substr(9);
                    std::cout << "Found version from control file: " << m_version << "\n";
                }
            }
            else if (line.compare(0, 14, "Architecture: ") == 0) {
                m_architecture = line.substr(14);
                std::cout << "Found architecture: " << m_architecture << "\n";
            }
            else if (line.compare(0, 12, "Maintainer: ") == 0) {
                m_maintainer = line.substr(12);
                std::cout << "Found maintainer: " << m_maintainer << "\n";
            }
            else if (line.compare(0, 13, "Description: ") == 0) {
                m_description = line.substr(13);
                std::cout << "Found description: " << m_description << "\n";
            }
            else if (line.compare(0, 8, "Source: ") == 0) {
                foundSource = true;
            }
        }
    }

    // 验证必要字段
    bool isValid = true;
    if (m_packageName.empty()) {
        std::cerr << _("Error: Package name not found") << "\n";
        isValid = false;
    }
    if (m_version.empty()) {
        std::cerr << _("Warning: Version not found, using default version 0.1.0") << "\n";
        m_version = "0.1.0";
    } else {
        std::cout << "Using version: " << m_version << "\n";
    }
    if (m_architecture.empty()) {
        if (foundSource) {
            m_architecture = "all";
            std::cout << "Using default architecture 'all' for source package\n";
        } else {
            std::cerr << _("Error: Architecture not found") << "\n";
            isValid = false;
        }
    }

    if (!isValid) {
        std::cerr << _("Control file validation failed") << "\n";
        return false;
    }

    std::cout << _("Control file parsed successfully") << "\n";
    return true;
}

bool LingmoPkgBuilder::copyDebianFiles(const std::filesystem::path& debianDir) {
    try {
        // 复制整个 debian 目录到临时目录
        std::filesystem::copy(debianDir, m_tempDir / "DEBIAN", 
            std::filesystem::copy_options::recursive);
        return true;
    } catch (const std::exception& e) {
        std::cerr << _("Copy file failed") << ": " << e.what() << "\n";
        return false;
    }
}

bool LingmoPkgBuilder::buildFromDirectory(const std::filesystem::path& sourceDir, 
                                  const std::string& outputPath) {
    try {
        // 直接使用源目录构造 LingmoPkgBuilder
        LingmoPkgBuilder builder(sourceDir);
        return builder.build(sourceDir);
    } catch (const std::exception& e) {
        std::cerr << _("Build failed") << ": " << e.what() << "\n";
        return false;
    }
}

bool LingmoPkgBuilder::copyArtifacts(const std::string& packageName) const {
    try {
        std::filesystem::create_directories(s_outputDir);
        
        // 复制所有非目录文件
        for (const auto& entry : std::filesystem::directory_iterator(m_tempDir.parent_path())) {
            if (!entry.is_directory()) {  // 只复制文件，不复制目录
                std::filesystem::copy(entry.path(), s_outputDir / entry.path().filename(),
                    std::filesystem::copy_options::update_existing);
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << _("Copy artifacts failed") << ": " << e.what() << "\n";
        return false;
    }
}

bool LingmoPkgBuilder::runCommand(const std::string& cmd) {
    int result = std::system(cmd.c_str());
    return result == 0;
}

bool LingmoPkgBuilder::checkBuildDependencies(const std::filesystem::path& sourceDir) {
#ifdef HAVE_UNISTD_H
    if (geteuid() != 0) {
        std::cerr << _("Error: Build dependency check requires root privileges") << "\n"
                  << _("Please run with sudo") << "\n";
        return false;
    }
#endif

    std::cout << _("Checking build dependencies...") << "\n";
    
    if (!runCommand("apt-get update")) {
        std::cerr << _("Error: Failed to update package list") << "\n";
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(sourceDir)) {
        if (!entry.is_directory()) continue;
        
        auto debianDir = entry.path() / "debian";
        if (!std::filesystem::exists(debianDir)) continue;

        std::cout << _("Checking dependencies for") << " " << entry.path().filename() << "...\n";
        
        std::string cmd = "apt build-dep -y ./" + entry.path().string();
        if (!runCommand(cmd)) {
            std::cerr << _("Error: Failed to install build dependencies for") << " " 
                     << entry.path().filename() << "\n";
            return false;
        }
    }

    std::cout << _("All build dependencies checked") << "\n";
    return true;
} 