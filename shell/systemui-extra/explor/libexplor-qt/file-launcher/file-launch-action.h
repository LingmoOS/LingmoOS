/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILELAUNCHACTION_H
#define FILELAUNCHACTION_H

#include <QAction>
#include <gio/gio.h>

#include "explor-core_global.h"

namespace Peony {

/*!
 * \brief The FileLaunchAction class
 * \details
 * This class is used to launch a file with a specific application info.
 * \todo
 * add error reprot GUI.
 */
class PEONYCORESHARED_EXPORT FileLaunchAction : public QAction
{
    Q_OBJECT
public:
    explicit FileLaunchAction(const QString &uri,
                              GAppInfo *app_info,
                              bool forceWithArg = false,
                              QObject *parent = nullptr);

    ~FileLaunchAction() override;
    const QString getUri();
    bool isDesktopFileAction();
    const QString getAppInfoName();
    const QString getAppInfoDisplayName();
    GAppInfo *gAppInfo() {
        return m_app_info;
    }
    /*!
     * \brief tryGetDesktopFilePath
     * try getting launch action's desktop file path, this may
     * return nullptr if app info is not a desktop app info.
     *
     * if you want to get more infomation about the app info,
     * try using g_app_info_get_xxx() which provided by gio.
     * \return
     * desktop file path, maybe nullptr
     */
    QString tryGetDesktopFilePath();

protected:
    bool isValid();
    void execFile();
    void execFileInterm();
    void preCheck();

public Q_SLOTS:
    void lauchFileSync(bool forceWithArg = false, bool skipDialog = true);
    void lauchFileAsync(bool forceWithArg = false, bool skipDialog = true);
    void lauchFilesAsync(const QStringList files, bool forceWithArg = false, bool skipDialog = true);

    bool isExcuteableFile(QString fileType);

private:
    bool launchAppWithDBus();
    bool launchAppWithAppMgr();
    bool launchDefaultAppWithUrl();
    bool launchAppWithSession();

    bool checkAppDisabled();

private:
    QString m_uri;
    bool m_is_desktop_file;
    GAppInfo *m_app_info;

    QIcon m_icon;
    QString m_info_name;
    QString m_info_display_name;

    QStringList m_executable_type = {"application/x-shellscript", "application/x-executable", "application/x-perl",
                                    "application/x-sharedlib", "text/x-python3", "application/javascript",
                                    "application/x-ruby"};

    /*!
     * \brief m_force_with_arg
     * \value true for forcing to execute action asynchonously.
     */
    bool m_force_with_arg = false;
};

}

#endif // FILELAUNCHACTION_H
