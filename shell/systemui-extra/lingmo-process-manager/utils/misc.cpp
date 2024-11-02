/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "misc.h"
#include <stdio.h>
#include <linux/magic.h>
#include <sys/vfs.h>
#include <sstream>
#include <fstream>
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>

namespace {
const char *cgroup_path = "/sys/fs/cgroup";
}

namespace misc {

bool string::startWith(const std::string &str, const std::string &prefix)
{
    if (str.length() < prefix.length()) {
        return false;
    }
    if (str.compare(0, prefix.length(), prefix) == 0) {
        return true;
    }
    return false;
}

std::vector<std::string> string::split(const std::string &str, const char sep)
{
    std::vector<std::string> stringList;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, sep)) {
        if (!token.empty()) {
            stringList.emplace_back(token);
        }
    }
    return stringList;
}

QStringList stl2qt::vectorString2QStringList(const std::vector<std::string> &vec)
{
    QStringList qStringList;
    for (const auto &value : vec) {
        qStringList.append(QString::fromStdString(value));
    }
    return qStringList;
};

QList<uint> stl2qt::intVector2QtUintList(const std::vector<int> &vecs)
{
    QList<uint> qlist;
    for (auto value : vecs) {
        qlist.append((uint)value);
    }
    return qlist;
}

std::string file::getFileNameFromPath(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.good()) {
        return std::string();
    }

    auto pathElements = misc::string::split(filePath, '/');
    if (pathElements.empty()) {
        return std::string();
    }

    return pathElements.back();
}

QByteArray file::fileMD5Hash(const std::string &filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
}

bool file::fileExists(const std::string &filePath)
{
    std::ifstream file(filePath);
    return file.good();
}

std::vector<std::string> qt2stl::qstringList2StdVectorString(const QStringList &qstrList)
{
    std::vector<std::string> vectorString;
    for (const auto &str : qstrList) {
        vectorString.push_back(str.toStdString());
    }

    return vectorString;
}

int version::cgroupVersion()
{
    // If you wonder how to detect which of these three modes is currently used,
    // use statfs() on /sys/fs/cgroup/. If it reports CGROUP2_SUPER_MAGIC in
    // its .f_type field, then you are in unified mode.
    // If it reports TMPFS_MAGIC then you are either in legacy or hybrid mode.
    // To distinguish these two cases, run statfs() again on /sys/fs/cgroup/unified/.
    // If that succeeds and reports CGROUP2_SUPER_MAGIC you are in hybrid mode,
    // otherwise not.
    struct statfs fs;
    statfs(cgroup_path, &fs);
    std::stringstream ss;
    ss << fs.f_type;
    // legacy or hybrid mode
    if (fs.f_type == TMPFS_MAGIC) {
        return 1;
    } else if (fs.f_type == CGROUP2_SUPER_MAGIC) {
        return 2;
    }

    qWarning() << "Unknown cgroup version.";
    return -1;
}

};
