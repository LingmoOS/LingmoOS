/*
 * Peony-Qt
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 */

#ifndef TRASHCLEANEDWATCHER_H
#define TRASHCLEANEDWATCHER_H

#include "file-watcher.h"
#include <QtConcurrent>
#include "explorer-core_global.h"

class QTimeLine;

namespace Peony {

class PEONYCORESHARED_EXPORT TrashCleanedWatcher : public QObject
{
    Q_OBJECT
public:
    static TrashCleanedWatcher*getInstance();
    TrashCleanedWatcher();
    ~TrashCleanedWatcher();

Q_SIGNALS:
    void updateTrashIcon();

protected Q_SLOTS:
    void startWatching();
    void startCheck();
    void checkRecycleBin();

private:
    std::shared_ptr<FileWatcher>  m_trash_watcher;
    QThread *m_syncThread = nullptr;
    QThread *m_watchThread = nullptr;
    QTimeLine *m_batchTimer = nullptr;
    int m_fileNum = -1;
};
}
#endif // TRASHCLEANEDWATCHER_H
