// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2019-2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "dirmodel.h"

#include <KDirLister>
#include <KIO/Job>

DirModel::DirModel(QObject *parent)
    : KDirSortFilterProxyModel(parent)
    , m_lister(new KDirLister(this))
{
    setSourceModel(&m_dirModel);
    m_dirModel.setDirLister(m_lister);
    connect(m_lister, QOverload<>::of(&KCoreDirLister::completed), this, &DirModel::isLoadingChanged);
    connect(m_lister, &KDirLister::jobError, this, [this](KIO::Job *job) {
        m_lastError = job->errorString();
        Q_EMIT lastErrorChanged();
    });
}

QVariant DirModel::data(const QModelIndex &index, int role) const
{
    if (!hasIndex(index.row(), index.column(), index.parent()))
        return {};

    switch (role) {
    case Roles::Name:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).name();
    case Url:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).url();
    case IconName:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).iconName();
    case IsDir:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).isDir();
    case IsLink:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).isLink();
    case FileSize:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).size();
    case MimeType:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).mimetype();
    case IsHidden:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).isHidden();
    case IsReadable:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).isReadable();
    case IsWritable:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).isWritable();
    case ModificationTime:
        return qvariant_cast<KFileItem>(KDirSortFilterProxyModel::data(index, KDirModel::FileItemRole)).time(KFileItem::ModificationTime);
    }

    return {};
}

QHash<int, QByteArray> DirModel::roleNames() const
{
    return {
        {Name, QByteArrayLiteral("name")},
        {Url, QByteArrayLiteral("url")},
        {IconName, QByteArrayLiteral("iconName")},
        {IsDir, QByteArrayLiteral("isDir")},
        {IsLink, QByteArrayLiteral("isLink")},
        {FileSize, QByteArrayLiteral("fileSize")},
        {MimeType, QByteArrayLiteral("mimeType")},
        {IsHidden, QByteArrayLiteral("isHidden")},
        {IsReadable, QByteArrayLiteral("isReadable")},
        {IsWritable, QByteArrayLiteral("isWritable")},
        {ModificationTime, QByteArrayLiteral("modificationTime")},
    };
}

QUrl DirModel::folder() const
{
    return m_lister->url();
}

void DirModel::setFolder(const QUrl &folder)
{
    if (folder != m_lister->url()) {
        m_dirModel.openUrl(folder);

        Q_EMIT folderChanged();
    }
}

bool DirModel::isLoading() const
{
    return !m_lister->isFinished();
}

bool DirModel::showDotFiles() const
{
    return m_lister->showHiddenFiles();
}

void DirModel::setShowDotFiles(bool showDotFiles)
{
    m_lister->setShowHiddenFiles(showDotFiles);
    m_lister->emitChanges();
    Q_EMIT showDotFilesChanged();
}

QString DirModel::nameFilter() const
{
    return m_lister->nameFilter();
}

void DirModel::setNameFilter(const QString &nameFilter)
{
    if (nameFilter != m_lister->nameFilter()) {
        m_lister->setNameFilter(nameFilter);
        m_lister->emitChanges();

        Q_EMIT nameFilterChanged();
    }
}

QStringList DirModel::mimeFilters() const
{
    return m_lister->mimeFilters();
}

void DirModel::setMimeFilters(const QStringList &mimeFilters)
{
    if (mimeFilters != m_lister->mimeFilters()) {
        QStringList newMimeFilters = mimeFilters;
        newMimeFilters.append(QStringLiteral("inode/directory"));

        m_lister->setMimeFilter(newMimeFilters);
        m_lister->emitChanges();

        Q_EMIT mimeFiltersChanged();
    }
}

void DirModel::resetMimeFilters()
{
    m_lister->clearMimeFilter();
    m_lister->emitChanges();
    Q_EMIT mimeFiltersChanged();
}

QString DirModel::lastError() const
{
    return m_lastError;
}
