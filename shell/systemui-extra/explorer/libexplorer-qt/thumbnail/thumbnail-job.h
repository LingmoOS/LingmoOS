/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef THUMBNAILJOB_H
#define THUMBNAILJOB_H

#include <QObject>
#include <QRunnable>
#include <memory>

#include "explorer-core_global.h"
#include "file-info.h"

namespace Peony {

class FileWatcher;

class PEONYCORESHARED_EXPORT ThumbnailJob : public QObject, public QRunnable
{
    Q_OBJECT
public:
    enum Type {
        Unknown = 0,
        Invalid = 1,
        Image = 2,
        Pdf = 3,
        ImagePdf = 4,
        Video = 5,
        Office = 6,
        Desktop = 7,
        CustomIcon = 8,
        Other
    };
    Q_ENUM (Type)

    explicit ThumbnailJob(const QString &uri, const std::shared_ptr<FileWatcher> watcher, QObject *parent = nullptr);
    ~ThumbnailJob();

public Q_SLOTS:
    void run() override;

    void setForceUpdate(bool force);
    bool isForceUpdate();

    void setType(Type type);
    Type type();

private:
    QString m_uri;
    std::weak_ptr<FileWatcher> m_watcher;
};

}

#endif // THUMBNAILJOB_H
