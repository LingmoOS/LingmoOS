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

#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>

#include "explor-core_global.h"

#include <gio/gio.h>

namespace Peony {

/*!
 * \brief The FileWatcher class
 * <br>
 * FileWatcher class is a wrapper of a set of GFileMonitor handles.
 * The most obvious difference between it and the ordinary GFileMonitor is that
 * it can dynamically track the monitoring directory. For example,
 * if your directory move to another path, the watcher will aslo change
 * its monitors. If you delete the path (or trash), it will be deleted
 * automaticly later.
 * </br>
 * \bug
 * FileWatcher can't monitor some special directory, such as a sftp:// server.
 * It will cause the model can not stay in sync with filesystem. This bug is the
 * gio's limitations of GFileMonitor.
 */
class PEONYCORESHARED_EXPORT FileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcher(QString uri = nullptr, QObject *parent = nullptr, bool isWatchMovesFlag=false);
    ~FileWatcher();
    /*!
     * \brief setMonitorChildrenChange
     * \param monitor_children_change
     * \details
     * For most case, we do not care wether a file internal changed event
     * in file manager (for example, the file's content changed).
     * Some special files, like files in computer:///, could use this signal
     * to monitor the changed event that not created or deleted
     * (for volume file handle in computer:///, it might be mount/unmount).
     */
    void setMonitorChildrenChange(bool monitor_children_change = true) {
        m_montor_children_change = monitor_children_change;
    }
    void startMonitor();
    void stopMonitor();

    void forceChangeMonitorDirectory(const QString &uri);

    const QString currentUri() {return m_uri;}

    /*!
     * \brief supportMonitor
     * \return
     * \details
     * Some file or directory doesn't support g_file_monitor* methods.
     * We can use this to ensure that if it was truely in monitoring.
     * If not, we might take over the handle of file change in our own
     * code.
     */
    bool supportMonitor() {
        return m_support_monitor;
    }

    void creatorMonitor();

Q_SIGNALS:
    void locationChanged(const QString &oldUri, const QString &newUri);
    void directoryDeleted(const QString &uri);
    void directoryUnmounted(const QString &uri);
    void fileCreated(const QString &uri);
    void fileDeleted(const QString &uri);
    void fileChanged(const QString &uri);
    void fileRenamed(const QString &oldUri, const QString &newUri);
    /*!
     * \brief requestUpdateDirectory
     * \note
     * only used in directory not support monitor
     */
    void requestUpdateDirectory();

    void thumbnailUpdated(const QString &uri);

public Q_SLOTS:
    void cancel();

protected:
    void prepare();

    static void file_changed_callback(GFileMonitor *monitor,
                                      GFile *file,
                                      GFile *other_file,
                                      GFileMonitorEvent event_type,
                                      FileWatcher *p_this);

    static void dir_changed_callback(GFileMonitor *monitor,
                                     GFile *file,
                                     GFile *other_file,
                                     GFileMonitorEvent event_type,
                                     FileWatcher *p_this);

    void changeMonitorUri(QString uri);

private:
    QString m_uri = nullptr;
    QString m_target_uri = nullptr;
    GFile *m_file = nullptr;
    GFileMonitor *m_monitor = nullptr;
    GFileMonitor *m_dir_monitor = nullptr;

    bool m_montor_children_change = false;

    GCancellable *m_cancellable = nullptr;

    /*!
     * \brief m_monitor_err
     * \deprecated use GErrorWrapper
     */
    GError *m_monitor_err = nullptr;
    /*!
     * \brief m_dir_monitor_err
     * \deprecated use GErrorWrapper
     */
    GError *m_dir_monitor_err = nullptr;

    gulong m_file_handle = 0;
    gulong m_dir_handle = 0;

    bool m_support_monitor = true;

    /* 文件和文件夹重名用先delete再create的方式(即GFileMonitorFlags使用G_FILE_MONITOR_NONE)时该标志值为false，
    文件和文件夹重名用rename的方式(即GFileMonitorFlags使用G_FILE_MONITOR_WATCH_MOVES)时该标志值为true */
    bool m_isWatchMovesFlag = false;
};

}

#endif // FILEWATCHER_H
