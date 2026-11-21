#include "lingmo_pkgbuild.h"
#include <iostream>
#include <filesystem>
#include <libintl.h>
#include <locale.h>

#define _(str) gettext(str)

void printUsage(const char* programName) {
    std::cout << _("Lingmo OS Package Build Tool") << "\n\n"
              << _("Usage:") << "\n"
              << "  " << programName << " [" << _("options") << "] <" << _("directory") << ">\n"
              << _("Options:") << "\n"
              << "  -h, --help     " << _("Show this help message") << "\n"
              << "  -o, --output   " << _("Specify output directory") << " (" << _("default") << ": pkg_out)\n"
              << "  -b, --build-dir " << _("Specify build directory") << " (" << _("default") << ": .build_deb_lingmo)\n"
              << "  -j, --jobs     " << _("Specify number of parallel builds") << " (" << _("default") << ": 1)\n"
              << "  --no-sign      " << _("Do not sign the package") << "\n"
              << "  -k, --key      " << _("Specify signing key") << "\n"
              << "  --no-deps      " << _("Skip build dependency check") << "\n"
              << "  -c, --clean    " << _("Clean build directory before and after build") << "\n"
              << _("Note: Build dependency check requires root privileges") << "\n";
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    bindtextdomain("lingmo-pkgbuild", "/usr/share/locale");
    textdomain("lingmo-pkgbuild");

    try {
        if (argc < 2) {
            printUsage(argv[0]);
            return 1;
        }

        std::filesystem::path sourceDir;
        std::filesystem::path outputDir = "pkg_out";
        std::filesystem::path buildDir = ".build_deb_lingmo";
        int threadCount = 1;
        bool sign = true;
        std::string signKey;
        bool checkDeps = true;  // 默认检查依赖
        bool clean = false;     // 默认不清理

        // 解析命令行参数
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                printUsage(argv[0]);
                return 0;
            } else if (arg == "-o" || arg == "--output") {
                if (++i >= argc) {
                    std::cerr << _("Error: Missing output directory argument") << "\n";
                    return 1;
                }
                outputDir = argv[i];
            } else if (arg == "-b" || arg == "--build-dir") {
                if (++i >= argc) {
                    std::cerr << _("Error: Missing build directory argument") << "\n";
                    return 1;
                }
                buildDir = argv[i];
            } else if (arg.substr(0, 2) == "-j") {
                // 处理 -j20 这样的格式
                std::string numStr = arg.substr(2);
                if (numStr.empty() && ++i < argc) {
                    numStr = argv[i];
                }
                try {
                    threadCount = std::stoi(numStr);
                    if (threadCount < 1) {
                        std::cerr << _("Error: Number of parallel builds must be greater than 0") << "\n";
                        return 1;
                    }
                } catch (const std::exception&) {
                    std::cerr << _("Error: Invalid number of parallel builds") << "\n";
                    return 1;
                }
            } else if (arg == "--jobs") {
                if (++i >= argc) {
                    std::cerr << _("Error: Missing parallel build count argument") << "\n";
                    return 1;
                }
                try {
                    threadCount = std::stoi(argv[i]);
                    if (threadCount < 1) {
                        std::cerr << _("Error: Number of parallel builds must be greater than 0") << "\n";
                        return 1;
                    }
                } catch (const std::exception&) {
                    std::cerr << _("Error: Invalid number of parallel builds") << "\n";
                    return 1;
                }
            } else if (arg == "--no-sign") {
                sign = false;
            } else if (arg == "-k" || arg == "--key") {
                if (++i >= argc) {
                    std::cerr << _("Error: Missing signing key argument") << "\n";
                    return 1;
                }
                signKey = argv[i];
            } else if (arg == "--no-deps") {
                checkDeps = false;
            } else if (arg == "-c" || arg == "--clean") {
                clean = true;
            } else if (arg[0] == '-' && arg != "-j") {
                std::cerr << _("Error: Unknown option") << " " << arg << "\n";
                return 1;
            } else {
                sourceDir = arg;
            }
        }

        if (sourceDir.empty()) {
            std::cerr << _("Error: Please specify source directory") << "\n";
            return 1;
        }

        if (!std::filesystem::exists(sourceDir)) {
            std::cerr << _("Error: Source directory does not exist") << ": " << sourceDir << "\n";
            return 1;
        }

        // 如果指定了清理选项，先清理构建目录
        if (clean) {
            LingmoPkgBuilder::cleanBuildDir();
        }

        // 设置全局配置
        LingmoPkgBuilder::setGlobalBuildDir(buildDir);
        LingmoPkgBuilder::setGlobalOutputDir(outputDir);
        LingmoPkgBuilder::setThreadCount(threadCount);
        LingmoPkgBuilder::setSignBuild(sign);
        if (!signKey.empty()) {
            LingmoPkgBuilder::setSignKey(signKey);
        }

        // 检查构建依赖
        if (checkDeps && !LingmoPkgBuilder::checkBuildDependencies(sourceDir)) {
            std::cerr << _("Build dependency check failed") << "\n";
            return 1;
        }

        // 遍历源码目录中的每个包目录
        bool allSuccess = true;
        for (const auto& entry : std::filesystem::directory_iterator(sourceDir)) {
            if (!entry.is_directory()) continue;

            std::cout << _("Building") << " \"" << entry.path().filename().string() << "\"...\n";
            if (!LingmoPkgBuilder::buildFromDirectory(entry.path(), outputDir.string())) {
                std::cerr << _("Failed to build") << " \"" << entry.path().filename().string() << "\"\n";
                allSuccess = false;
            }
        }

        if (!allSuccess) {
            std::cerr << _("Some packages failed to build") << "\n";
            return 1;
        }

        std::cout << _("All packages built successfully") << "\n";
        std::cout << _("Build artifacts are located at") << ": " << std::filesystem::absolute(outputDir) << "\n";

        // 如果指定了清理选项，构建完成后再次清理
        if (clean) {
            LingmoPkgBuilder::cleanBuildDir();
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << _("Error") << ": " << e.what() << "\n";
        return 1;
    }
} 