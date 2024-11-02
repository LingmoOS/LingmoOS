/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */


#ifndef METADATAEMBLEMPROVIDER_H
#define METADATAEMBLEMPROVIDER_H

#include <QObject>
#include <QThread>
#include "explor-core_global.h"
#include "emblem-provider.h"
#include "file-meta-info.h"

namespace Peony {

class PEONYCORESHARED_EXPORT MetadataEmblemProvider : public EmblemProvider
{
    Q_OBJECT
public:
    static MetadataEmblemProvider *getInstance();

    const QString emblemKey() override;

    QStringList getFileEmblemIcons(const QString &uri) override;

Q_SIGNALS:
    std::shared_ptr<Peony::FileMetaInfo> requestDupMetaInfo(const QString &uri); //deprecated

private:
    explicit MetadataEmblemProvider(QObject *parent = nullptr);

private Q_SLOTS:
    std::shared_ptr<Peony::FileMetaInfo> getDupMetaInfo(const QString &uri); //deprecated
};

class MetadataDupJob : public QThread
{
    Q_OBJECT
public:
    explicit MetadataDupJob(const QString &uri, QObject *parent = nullptr);

    void run() override;

    std::shared_ptr<Peony::FileMetaInfo> dupMetaInfo() const;

Q_SIGNALS:
    std::shared_ptr<Peony::FileMetaInfo> requestDupMetaInfo(const QString &uri);

private:
    QString m_uri;
    std::shared_ptr<Peony::FileMetaInfo> m_dupMetaInfo = nullptr;
};

}

#endif // METADATAEMBLEMPROVIDER_H
