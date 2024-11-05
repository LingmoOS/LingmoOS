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

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMutex>

#include "explorer-core_global.h"

namespace Peony {

/*!
 * \brief The BookMarkManager class
 * \details
 * This class is use to manage bookmarks of explorer.
 * You can add/remove custom bookmark at explorer's side bar.
 */
class PEONYCORESHARED_EXPORT BookMarkManager : public QObject
{
    Q_OBJECT
public:
    static BookMarkManager *getInstance();

    const QStringList getCurrentUris() {
        return m_uris;
    }
    bool isLoaded() {
        return m_is_loaded;
    }

   bool existsInBookMarks(const QString &uri);

Q_SIGNALS:
    void urisLoaded();
    void bookMarkAdded(const QString &uri, bool successed);
    void bookMarkRemoved(const QString &uri, bool successed);
    void bookmarkChanged(const QString oldUri, const QString newUri);

public Q_SLOTS:
    void addBookMark(const QString &uri);
    void removeBookMark(const QString &uri);
    void renameBookmark(const QString oldUri, const QString newUri);
    void removeBookMark(const QStringList &uris);

private:
    explicit BookMarkManager(QObject *parent = nullptr);
    ~BookMarkManager();
    void addBookMarkPrivate(const QString &uri);
    void removeBookMarkPrivate(const QString &uri);

    QStringList m_uris;
    QSettings *m_book_mark = nullptr;
    bool m_is_loaded = false;
    QMutex m_mutex;
};

}

#endif // BOOKMARKMANAGER_H
