/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef SHAREEMBLEMPROVIDER_H
#define SHAREEMBLEMPROVIDER_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <memory>
#include "explor-core_global.h"
#include "emblem-provider.h"

#define SHARE_EMBLEMS   "share-emblems"
namespace Peony {
class ShareEmblemProvider : public EmblemProvider
{
    Q_OBJECT
public:
    static ShareEmblemProvider *getInstance();

    const QString emblemKey() override;

    QStringList getFileEmblemIcons(const QString &uri) override;

private:
    explicit ShareEmblemProvider(QObject *parent = nullptr);
};

class FileXattrInfo
{
public:
    static std::shared_ptr<FileXattrInfo> fromUri(const QString &uri);

    FileXattrInfo(const QString &uri);

    void setXattrInfoString(const QString &key, const QString &value, bool syncToFile = true);

    const QString getXattrInfoString(const QString &key);

    void removeXattrInfo(const QString &key);

private:
    QString m_uri;
    QHash<QString, QVariant> m_hash;
};
}

#endif // SHAREEMBLEMPROVIDER_H
