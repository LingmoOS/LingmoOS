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

#ifndef PATHBARMODEL_H
#define PATHBARMODEL_H

#include "explorer-core_global.h"
#include <QStringListModel>
#include <QHash>
#include <memory>

namespace Peony {

class FileInfo;
class FileEnumerator;

/*!
 * \brief The PathBarModel class
 * \details
 * PathBarModel is desgin for path completions.
 * This model would cache a directory driect children when the target uri
 * was set.
 * \note
 * A completion is theoretically responsive, so the enumeration of model
 * items should be as fast as possible.
 * It must be fast and lightweight enough to keep the ui-frequency.
 * For now, it performs well at local file system, but not good enough at
 * remote fs.
 */
class PEONYCORESHARED_EXPORT PathBarModel : public QStringListModel
{
    Q_OBJECT
public:
    explicit PathBarModel(QObject *parent = nullptr);
    QString findDisplayName(const QString &uri);
    QString currentDirUri() {
        return m_current_uri;
    }

Q_SIGNALS:
    void updated();

public Q_SLOTS:
    void setRootPath(const QString &path, bool force = false);
    void setRootUri(const QString &uri, bool force = false);

private:
    QString m_current_uri = nullptr;
    QHash<QString, QString> m_uri_display_name_hash;

    std::shared_ptr<FileInfo> m_info;
    QList<std::shared_ptr<FileInfo>> m_childrens;

    FileEnumerator *m_enumerator = nullptr;
};

}

#endif // PATHBARMODEL_H
