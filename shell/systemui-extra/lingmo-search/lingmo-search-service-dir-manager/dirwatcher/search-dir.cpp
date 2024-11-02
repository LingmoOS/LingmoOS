/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "search-dir.h"
#include "config.h"
#include "volume-manager.h"
#include <QFile>

SearchDir::SearchDir(const QString &path, bool generateBlackList)
{
    if (!QFile::exists(path)) {
        m_error = NotExists;
    } else {
        m_path = path;
        if(generateBlackList) {
            this->handleBlackListGenerate();
        }
    }
}

bool SearchDir::operator ==(const SearchDir &rhs) const
{
    return (m_path == rhs.m_path);
}

SearchDir::ErrorInfo SearchDir::error()
{
    return m_error;
}

QString SearchDir::errorString()
{
    switch (m_error) {
    case ErrorInfo::Successful:
        return "Add search dir successful";
    case ErrorInfo::Duplicated:
        return "Path or its parent dir has been added";
    case ErrorInfo::UnderBlackList:
        return "Path is or under blacklist";
    case ErrorInfo::RepeatMount1:
        return "Path is in repeat mounted devices and another path which is in the same device has been indexed";
    case ErrorInfo::RepeatMount2:
        return "Another path which is in the same device has been indexed";
    case ErrorInfo::NotExists:
        return "Path is not exists";
    }
    return QString();
}

QString SearchDir::getPath() const
{
    return m_path;
}

void SearchDir::setBlackList(const QStringList &blackList)
{
    m_blackList = blackList;
}

QStringList SearchDir::getBlackList() const
{
    return m_blackList;
}

void SearchDir::generateBlackList()
{
    m_blackList.clear();
    this->handleBlackListGenerate();
}

QStringList SearchDir::blackListOfDir(const QString &dirPath)
{
    //手动刷新挂载信息
    VolumeManager::self()->refresh();
    QStringList blackListOfDir;
    for (const QStringList & mountPoints: VolumeManager::self()->getDuplicates()) {
        QString topRepeatedMountPoint;
        for (const QString &mountPoint : mountPoints) {
            //该目录下是否有两个文件夹（有可能为父子关系）是重复挂载的关系
            if (mountPoint.startsWith(dirPath + "/") or mountPoint == dirPath) {
                if (topRepeatedMountPoint.isEmpty()) {
                    topRepeatedMountPoint = mountPoint;
                    continue;
                }
                //重复挂载时保留最上层的挂载点
                if (topRepeatedMountPoint.startsWith(mountPoint + "/")) {
                    blackListOfDir.append(topRepeatedMountPoint);
                    topRepeatedMountPoint = mountPoint;
                } else {
                    blackListOfDir.append(mountPoint);
                }
            }
        }
    }

    blackListOfDir << handleSubVolumes4SingleDir(dirPath);
    return blackListOfDir;
}

void SearchDir::handleBlackListGenerate()
{
    //手动刷新挂载信息
    VolumeManager::self()->refresh();
    QStringList searchDirs = Config::self()->searchDirs();

    //目录已被索引（根目录被添加过直接返回）
    for (const QString searchDir : searchDirs) {
        if (searchDir == m_path || searchDir == "/") {
            m_error = Duplicated;
            return;
        }
    }

    //根目录特殊处理
    if (m_path == "/") {
        m_blackList << Config::self()->globalBlackList() << searchDirs;
        for (const QStringList &mountPoints: VolumeManager::self()->getDuplicates()) {
            QStringList repeatMountPoints = mountPoints;
            if (mountPoints.contains("/")) {
                repeatMountPoints.removeAll("/");
                m_blackList << repeatMountPoints;
                continue;
            }
            bool excludeAll(false);
            for (const QString &mountPoint : mountPoints) {
                for (const QString &searchDir : searchDirs) {
                    //之前已索引重复挂载设备挂载点或其子目录，则排除其他目录
                    if (searchDir.startsWith(mountPoint + "/") || searchDir == mountPoint) {
                        repeatMountPoints.removeAll(mountPoint);
                        break;
                    }
                    //重复挂载点在已索引目录下,该挂载点全排除
                    if (mountPoint.startsWith(searchDir + "/")) {
                        excludeAll = true;
                        break;
                    }
                }
                if (excludeAll) {
                    m_blackList << repeatMountPoints;
                    break;
                }
            }
            //不需要全排除且没有需要特别保留的挂载点，默认留第一个
            if (!excludeAll && repeatMountPoints == mountPoints) {
                repeatMountPoints.removeFirst();
            }
            m_blackList << repeatMountPoints;
        }
        m_blackList.append(handleSubVolumes4SingleDir("/"));
        m_blackList.removeDuplicates();
        return;
    }

    //处理要添加索引的路径与全局黑名单中路径为父子关系的情况
    for (const QString& blackListPath : Config::self()->globalBlackList()) {
        if (m_path.startsWith(blackListPath + "/") or m_path == blackListPath) {
            m_error = UnderBlackList;
            return;
        }
        if (blackListPath.startsWith(m_path + "/")) {
            m_blackList.append(blackListPath);
        }
    }

    //重复挂载情况
    for (const QStringList & mountPoints: VolumeManager::self()->getDuplicates()) {
        QString topRepeatedMountPoint;
        for (const QString &mountPoint : mountPoints) {
            if (mountPoint.startsWith(m_path + "/") or mountPoint == m_path) {
                if (topRepeatedMountPoint.isEmpty()) {
                    topRepeatedMountPoint = mountPoint;
                    continue;
                } else if (topRepeatedMountPoint.startsWith(mountPoint)) {
                    m_blackList.append(topRepeatedMountPoint);
                    topRepeatedMountPoint = mountPoint;
                } else {
                    m_blackList.append(mountPoint);
                }
            }
        }

        //排除要添加的目录为某设备的重复挂载目录，并且之前已索引过该设备其他挂载目录或其父目录的情况
        bool pathToBeAddedIsRepeatedDevice = false;
        bool pathToBeAddedHasRepeatedDevice = false;
        bool addedPathIsRepeatedDevice = false;
        bool addedPathHasRepeatedDevice = false;
        QString addedRelativeDir;
        QString repeatedDir;
        for (const QString &addedPath : searchDirs) {
            for (const QString &mountPoint : mountPoints) {

                //要添加索引路径在重复挂载设备路径下(1)
                if (m_path.startsWith(mountPoint + "/") or mountPoint == m_path) {
                    repeatedDir = mountPoint;
                    pathToBeAddedIsRepeatedDevice = true;
                }
                //重复挂载设备路径在要添加索引路径下(2)
                if (mountPoint.startsWith(m_path + "/")) {
                    repeatedDir = mountPoint;
                    pathToBeAddedHasRepeatedDevice = true;
                }

                //已索引路径在重复挂载设备路径下(3)
                if (addedPath.startsWith(mountPoint + "/") or mountPoint == addedPath) {
                    addedRelativeDir = addedPath;
                    addedRelativeDir.remove(0, mountPoint.length());
                    addedPathIsRepeatedDevice = true;
                }
                //重复挂载设备路径在已索引路径下(4)
                if (mountPoint.startsWith(addedPath + "/")) {
                    addedPathHasRepeatedDevice = true;
                }

                //(1)(4)直接返回
                if (pathToBeAddedIsRepeatedDevice and addedPathHasRepeatedDevice) {
                    m_error = RepeatMount1;
                    return;
                }
                //(2)(4)将要添加索引目录相应的重复挂载路径添加到黑名单
                if (pathToBeAddedHasRepeatedDevice and addedPathHasRepeatedDevice) {
                    m_blackList.append(repeatedDir);
                    break;
                }
                //(1)(3)将已索引路径的前缀替换为要添加路径的前缀（前缀为mountPoint），判断替换后路径是否在要索引路径下，如果是则返回，否则将替换后路径添加到黑名单
                if (pathToBeAddedIsRepeatedDevice and addedPathIsRepeatedDevice) {
                    QString pathAfterReplace = repeatedDir + addedRelativeDir;
                    if (m_path.startsWith(pathAfterReplace) or m_path == pathAfterReplace) {
                        m_error = RepeatMount2;
                        return;
                    } else {
                        m_blackList.append(pathAfterReplace);
                        break;
                    }
                }
                //(2)(3)将替换前缀后的已索引路径添加到黑名单
                if (pathToBeAddedHasRepeatedDevice and addedPathIsRepeatedDevice) {
                    m_blackList.append(repeatedDir + addedRelativeDir);
                    break;
                }
            }
        }
    }

    //处理自动挂载子卷下的目录
    for (const Volume &volume : VolumeManager::self()->volumesHaveSubVolumes()) {
        QMap<QString, QString> subVolumeInfo = volume.subVolumes();
        for (auto it = subVolumeInfo.constBegin(); it != subVolumeInfo.constEnd(); it++) {
            QString subMountPoint = it.key();
            for (const QString &duplicateMountPoint : volume.mountPoints()) {
                if (subVolumeInfo.keys().contains(duplicateMountPoint)) {
                    continue;
                }
                QString spec = duplicateMountPoint + it.value(); //子卷对应目录
                //要添加目录下存在子卷(要添加/data，但挂到/home的/data/home是子卷），若添加了/home则将/data/home排除
                if (spec.startsWith(m_path + "/")) {
                    for (QString &searchDir : searchDirs) {
                        if (searchDir == subMountPoint || subMountPoint.startsWith(searchDir + "/")) {
                            m_blackList << spec;
                        }
                        if (searchDir.startsWith(subMountPoint + "/")) {
                            m_blackList << searchDir.replace(0, subMountPoint.length(), spec);
                        }
                    }

                }

                //要添加的目录是子卷或在子卷下（/data/home or /data/home/xxx）
                if (m_path.startsWith(spec + "/") || m_path == spec) {
                    for (QString &searchDir : searchDirs) {
                        //已添加挂载点或其上层目录
                        if (subMountPoint.startsWith(searchDir + "/") || searchDir == subMountPoint) {
                            m_error = RepeatMount1;
                            return;
                        }
                        //已添加挂载点下其他目录
                        if (searchDir.startsWith(subMountPoint + "/")) {
                            QString tmp = searchDir;
                            tmp.replace(0, subMountPoint.length(), spec);
                            if (tmp == m_path) {
                                m_error = RepeatMount2;
                                return;
                            } else if (tmp.startsWith(m_path + "/")) {//已添加的子卷子目录替换前缀后在要添加目录下
                                m_blackList << tmp;
                            }
                        }
                    }
                }

                //要添加的目录是挂载点或在挂载点下（/home or /home/xxx）
                if (m_path.startsWith(subMountPoint + "/") || m_path == subMountPoint) {
                    for (QString &searchDir : searchDirs) {
                        //已添加子卷或其上层目录
                        if (spec.startsWith(searchDir + "/") || searchDir == spec) {
                            m_error = RepeatMount1;
                            return;
                        }
                        //已添加子卷下其他目录
                        if (searchDir.startsWith(spec + "/")) {
                            QString tmp = searchDir;
                            tmp.replace(0, spec.length(), subMountPoint);
                            if (tmp == m_path) {
                                m_error = RepeatMount2;
                                return;
                            } else if (tmp.startsWith(m_path + "/")) {//已添加的子卷子目录替换前缀后在要添加目录下
                                m_blackList << tmp;
                            }
                        }
                    }
                }
            }
        }

    }

    //要添加目录下存在已索引目录
    for (const QString &searchDir : searchDirs) {
        if (m_path.startsWith(searchDir + "/")) {
            m_error = Duplicated;
            return;
        }
        if (searchDir.startsWith(m_path + "/")) {
            m_blackList.append(searchDir);
        }
    }
    m_blackList.removeDuplicates();
}

QStringList SearchDir::handleSubVolumes4SingleDir(const QString &searchDir) {
    QStringList blackList;
    for (const Volume &volume : VolumeManager::self()->volumesHaveSubVolumes()) {
        QMap<QString, QString> subVolumeInfo = volume.subVolumes();
        for (auto it = subVolumeInfo.constBegin(); it != subVolumeInfo.constEnd(); it++) {
            QString subMountPoint = it.key();

            for (const QString &duplicateMountPoint : volume.mountPoints()) {
                if (subVolumeInfo.keys().contains(duplicateMountPoint)) {
                    continue;
                }
                QString spec = duplicateMountPoint + it.value(); //子卷对应目录

                if (searchDir == "/") {
                    blackList << spec;
                } else {
                    //排除搜索列表指定多个目录时子卷会重复包含的情况，比如同时指定/home和/data/home
                    QString tmp = searchDir;
                    if (searchDir.startsWith(subMountPoint + "/")) {
                        blackList << tmp.replace(0, subMountPoint.length(), spec);
                    }
                    //要添加目录下存在子卷(要添加/data，但挂到/home的/data/home是子卷），若添加了/home则将/data/home排除
                    if (searchDir.startsWith(spec + "/")) {
                        blackList << tmp.replace(0, spec.length(), subMountPoint);
                    }
                    if (spec.startsWith(searchDir + "/")) {
                        if (searchDir == subMountPoint || subMountPoint.startsWith(searchDir + "/")) {
                            blackList << spec;
                        }
                    }
                }
            }
        }
    }
    return blackList;
}
