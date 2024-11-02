/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkysysinfo.hpp"
#include <fstream>

namespace KDK_SYSTEM{

static std::string getValueFromFile(std::ifstream &file, std::string key)
{
    std::string value;
    if (!file.is_open())
        return value;
    
    std::string buf;
    while (file.eof() != true)
    {
        std::getline(file, buf);
        if (buf.compare(0, key.length(), key) == 0)
        {
            value = buf.substr(key.length() + 1);
            break;
        }
    }

    return value;
}

static std::string stripString(std::string str, char ch)
{
    uint startPos = str.find_first_not_of(ch);
    uint endPos = str.find_last_not_of(ch);
    return str.substr(startPos, endPos - startPos + 1);
}

std::string getSystemArchitecture()
{
    std::string architecture;
    std::ifstream file("/proc/osinfo", std::ios::in);
    if (!file.is_open())
        return architecture;
    
    architecture = getValueFromFile(file, "Architecture");
    architecture = stripString(architecture, '\t');

    file.close();
    return architecture;
}

std::string getSystemName()
{
    std::string sysName;
    std::ifstream file("/etc/os-release", std::ios::in);
    if (!file.is_open())
        return sysName;

    sysName = getValueFromFile(file, "NAME");
    sysName = stripString(sysName, '\"');

    file.close();
    return sysName;
}

std::string getSystemVersion(bool verbose)
{
    std::string systemVersion;
    if (verbose)
    {
        std::ifstream kyinfo("/etc/.kyinfo", std::ios::in);
        if (!kyinfo.is_open())
            return systemVersion;

        systemVersion = getValueFromFile(kyinfo, "milestone");

        kyinfo.close();
    }
    else
    {
        std::ifstream file("/etc/os-release", std::ios::in);
        if (!file.is_open())
            return systemVersion;
        
        systemVersion = getValueFromFile(file, "VERSION");
        systemVersion = stripString(systemVersion, '\"');
        file.close();
    }
    return systemVersion;
}

bool getSystemActivationStatus()
{

}

std::string getSystemSerialNumber()
{
    std::string serialNumber;
    std::ifstream file("/etc/.kyinfo", std::ios::in);
    if (! file.is_open())
        return serialNumber;
    
    serialNumber = getValueFromFile(file, "key");
    file.close();
    return serialNumber;
}

std::string getKernelVersion()
{
    std::string kernelVersion;
    std::ifstream file("/proc/version", std::ios::in);
    if (! file.is_open())
        return kernelVersion;
    
    std::string buf;
    std::getline(file, buf);
    size_t pos = buf.find(' ', 0);
    pos = buf.find(' ', pos + 1);
    size_t endpos = buf.find(' ', pos + 1);
    kernelVersion = buf.substr(pos + 1, endpos - pos - 1);

    file.close();
    return kernelVersion;
}

std::string getCurrentUserName()
{
    
}

}
