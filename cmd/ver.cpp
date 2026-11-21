#include "os_release.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <getopt.h>

// 从文件中读取构建版本号
std::string getBuildVersion(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string buildVersion;
    if (file.is_open()) {
        std::getline(file, buildVersion);
        file.close();
    } else {
        buildVersion = "Unknown";
    }
    return buildVersion;
}

// 获取当前时间戳
std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return std::string(buffer);
}

// 显示版权信息
void printCopyright() {
    std::cout << "Copyright (C) 2025 Lingmo Open Source Community. All rights reserved.\n";
}

void printHelp() {
    std::cout << "Usage: ver [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -a          Display all system version information.\n";
    std::cout << "  -n          Display only the system name (PRETTY_NAME).\n";
    std::cout << "  -v          Display only the system version (VERSION_ID).\n";
    std::cout << "  -b          Display only the build version.\n";
    std::cout << "  -t          Display the current timestamp.\n";
    std::cout << "  -h          Display this help message.\n";
}

int main(int argc, char* argv[]) {
    OSRelease osRelease;

    std::string name = osRelease.get("PRETTY_NAME");
    std::string version = osRelease.get("VERSION_ID");
    std::string buildVersion = getBuildVersion("/etc/system/currentVersion");
    std::string timestamp = getCurrentTimestamp();

    int opt;
    while ((opt = getopt(argc, argv, "anvbth")) != -1) {
        switch (opt) {
            case 'a':
                std::cout << "Lingmo OS [Version ";
                std::cout << name << " " << version << " " << buildVersion << "]" << std::endl;
                printCopyright();
                return 0;
            case 'n':
                std::cout << name << "\n";
                return 0;
            case 'v':
                std::cout << version << "\n";
                return 0;
            case 'b':
                std::cout << buildVersion << "\n";
                return 0;
            case 't':
                std::cout << "Timestamp: " << timestamp << "\n";
                return 0;
            case 'h':
                printHelp();
                return 0;
            default:
                printHelp();
                return 1;
        }
    }

    // 默认行为（无选项指定）
    std::cout << "Lingmo OS [Version ";
    std::cout << name << " " << version << " " << buildVersion << "]" << std::endl;
    printCopyright();

    return 0;
}

