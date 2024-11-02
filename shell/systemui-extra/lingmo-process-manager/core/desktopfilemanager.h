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

#ifndef DESKTOPFILEMANAGER_H
#define DESKTOPFILEMANAGER_H

#include <QObject>
#include <QFuture>
#include <QMutex>
#include <QMap>
#include <tuple>
class DesktopFileManager : public QObject
{
    Q_OBJECT
public:
    explicit DesktopFileManager(QObject *parent = nullptr);

    std::string syncGetDesktopFileByPid(int pid);
    std::vector<std::string> desktopFilesWithName(const std::string &desktopName);
    //todo: async

private:
    using FindDesktopFilePredicate = std::function<bool (std::tuple<QString, QStringList>)>;
    using DesktopExecMap = QMap<QString, std::tuple<QString, QStringList>>;

    void loadDesktopFiles();
    QStringList desktopFilePaths() const;
    void updateDesktopExecsInfo(const QString &path);
    // exec, args
    std::tuple<QString, QStringList> getExecByDesktopFile(const QString &desktopFile) const;
    QString desktopFilePath(const QString &desktopFileName) const;
    QString findDesktopFileFromLocalCache(const int &pid);
    QString findKmreAppDesktopFileByCmdline(const QString &cmdline);
    QString findKmreFullDesktopFileByCmdline(const QString &cmdline);
    QString findDesktopFileByCompareExec(const QString &cmdline, FindDesktopFilePredicate findDesktopFilePredicate);
    QString findDesktopFileByEqualExec(const QString &cmdline);
    QString findDesktopFileByContainsStartArgs(const QString &cmdline);
    QString findDesktopFileByContainsExec(const QString &cmdline);
    QString findDesktopFileByDpkgCommand(const int &pid);
    QString binaryPathFromCmdline(const QString &cmdline);
    QString findPackageNameFromBinaryPath(const QString &binaryPath);
    QString findDesktopFileFromPackageName(const QString &packageName, const QString &cmdline);
    bool desktopFileNoDisplay(const QString &desktopFile);
    QString desktopEntryFromDesktopFile(const QString &desktopFile, const QString &key);
    QStringList desktopFileListFromPackageName(const QString &packageName);
    QString bestDesktopFileFromMultDesktopFileLists(const QString &cmdline, const QStringList &desktopFileLists);
    QString findDesktopFileByEqualStartArg(const QString &cmdline);
    QString findDesktopFileFromPid(int pid);
    QString readGioLaunchEnvironFromFile(const std::string &file);
    DesktopExecMap standardPathDesktopFileExecMap();
    DesktopExecMap autoStartPathDesktopFileExecMap();

private:
    QFuture<void> m_loadDesktopFilesFuture;
    QMutex m_mutex;
    // desktopfile, exec, args
    DesktopExecMap m_desktopExecs;
    const QString kmreAppMainWindow = "/usr/bin/lingmo-kmre-window";
    const QString polkitWindowPidCmdline = "lingmo-polkit/polkit-lingmo-authentication-agent-1";

    const QStringList terminalLists = {
        "mate-terminal",
        "konsole",
        "lxterminal",
        "terminator",
        "tilda,"
        "guake",
        "yakuake",
        "terra",
        "rxvt",
        "sakura",
        "terminology"
    };
};

#endif // DESKTOPFILEMANAGER_H
