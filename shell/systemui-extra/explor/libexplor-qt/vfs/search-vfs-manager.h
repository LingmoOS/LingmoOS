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

#ifndef SEARCHVFSMANAGER_H
#define SEARCHVFSMANAGER_H

#include <QObject>
#include <QHash>
#include <QMutex>

namespace Peony {

class SearchVFSManager : public QObject
{
    Q_OBJECT
public:
    static SearchVFSManager *getInstance();

public Q_SLOTS:
    void clearHistory();
    /*!
     * \brief clearHistoryOne
     * \param searchUri
     * \details
     * if we refresh the directory, we should clean the history of the
     * directory and search again.
     */
    void clearHistoryOne(const QString &searchUri);
    void addHistory(const QString &searchUri, const QStringList &results);
    bool hasHistory(const QString &serachUri);
    QStringList getHistroyResults(const QString &searchUri) ;

private:
    explicit SearchVFSManager(QObject *parent = nullptr);
    ~SearchVFSManager();

    QMutex m_mutex;
    QHash<QString, QStringList> m_search_dir_results_hash;
};

}

#endif // SEARCHVFSMANAGER_H
