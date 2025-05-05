#include "os_release.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

OSRelease::OSRelease() {
    load();
}

void OSRelease::load() {
    std::ifstream file("/etc/os-release");
    std::string line;
    while (std::getline(file, line)) {
        parseLine(line);
    }
}

void OSRelease::parseLine(const std::string& line) {
    std::istringstream iss(line);
    std::string key, value;
    if (std::getline(iss, key, '=') && std::getline(iss, value)) {
        auto newEnd = std::remove(value.begin(), value.end(), '"'); // 获取新结束位置
        value.erase(newEnd, value.end()); // 删除从新结束位置到原结束位置的内容
        data[key] = value;
    }
}

std::string OSRelease::get(const std::string& key) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "Unknown";
}

