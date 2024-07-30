/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "PagesModel.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>
#include <KNSCore/Entry>

#include "PageDataObject.h"

PagesModel::PagesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> PagesModel::roleNames() const
{
    static QHash<int, QByteArray> roles{{TitleRole, "title"},
                                        {DataRole, "data"},
                                        {IconRole, "icon"},
                                        {FileNameRole, "fileName"},
                                        {HiddenRole, "hidden"},
                                        {FilesWriteableRole, "filesWriteable"}};
    return roles;
}

int PagesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_pages.count();
}

QVariant PagesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent)) {
        return QVariant{};
    }

    auto data = m_pages.at(index.row());
    switch (role) {
    case TitleRole:
        return data->value(QStringLiteral("title"));
    case DataRole:
        return QVariant::fromValue(data);
    case IconRole:
        return data->value(QStringLiteral("icon"));
    case FileNameRole:
        return data->fileName();
    case HiddenRole:
        return m_hiddenPages.contains(data->fileName());
    case FilesWriteableRole:
        return m_writeableCache[data->fileName()];
    default:
        return QVariant{};
    }
}

void PagesModel::classBegin()
{
}

void PagesModel::componentComplete()
{
    QHash<QString, QString> files;

    const auto directories = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    for (const auto &directory : directories) {
        QDir dir{directory};
        const auto entries = dir.entryInfoList({QStringLiteral("*.page")}, QDir::NoDotAndDotDot | QDir::Files);
        for (const auto &entry : entries) {
            const FilesWriteableStates entryState = entry.isWritable() ? AllWriteable : NotWriteable;
            if (!files.contains(entry.fileName())) {
                files.insert(entry.fileName(), dir.relativeFilePath(entry.fileName()));
                m_writeableCache.insert(entry.fileName(), entryState);
            } else {
                FilesWriteableStates &state = m_writeableCache[entry.fileName()];
                if ((state == NotWriteable && entryState == AllWriteable) || (state == AllWriteable && entryState == NotWriteable)) {
                    state = LocalChanges;
                }
            }
        }
    }

    beginResetModel();
    for (auto itr = files.begin(); itr != files.end(); ++itr) {
        auto config = KSharedConfig::openConfig(itr.value(), KConfig::CascadeConfig, QStandardPaths::AppDataLocation);

        auto page = new PageDataObject{config, this};
        page->load(*config, QStringLiteral("page"));

        connect(page, &PageDataObject::saved, this, [this, page]() {
            if (m_writeableCache[page->fileName()] == NotWriteable) {
                m_writeableCache[page->fileName()] = LocalChanges;
                auto i = m_pages.indexOf(page);
                Q_EMIT dataChanged(index(i), index(i), {FilesWriteableRole});
            }
        });
        connect(page, &PageDataObject::valueChanged, this, [this, page] {
            auto i = m_pages.indexOf(page);
            Q_EMIT dataChanged(index(i), index(i), {TitleRole, IconRole});
        });
        connect(page, &PageDataObject::loaded, this, [this, page] {
            auto i = m_pages.indexOf(page);
            Q_EMIT dataChanged(index(i), index(i), {TitleRole, IconRole});
        });
        m_pages.append(page);
    }
    sort();
    endResetModel();
}

void PagesModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)

    if (m_pageOrder.isEmpty()) {
        return;
    }

    Q_EMIT layoutAboutToBeChanged({QPersistentModelIndex()}, QAbstractItemModel::VerticalSortHint);
    auto last = std::stable_partition(m_pages.begin(), m_pages.end(), [this](PageDataObject *page) {
        return m_pageOrder.contains(page->fileName());
    });
    std::sort(m_pages.begin(), last, [this](PageDataObject *left, PageDataObject *right) {
        return m_pageOrder.indexOf(left->fileName()) < m_pageOrder.indexOf(right->fileName());
    });
    std::transform(last, m_pages.end(), std::back_inserter(m_pageOrder), [](PageDataObject *page) {
        return page->fileName();
    });
    if (last != m_pages.end()) {
        Q_EMIT pageOrderChanged();
    }
    changePersistentIndex(QModelIndex(), QModelIndex());
    Q_EMIT layoutChanged();
}

PageDataObject *PagesModel::addPage(const QString &baseName, const QVariantMap &properties)
{
    int counter = 0;
    const QString extension = QStringLiteral(".page");
    QString fileName = baseName + extension;
    while (m_writeableCache.contains(fileName)) {
        fileName = baseName + QString::number(++counter) + extension;
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig(fileName, KConfig::CascadeConfig, QStandardPaths::AppDataLocation);
    auto page = new PageDataObject(config, this);
    page->load(*config, QStringLiteral("page"));

    for (auto itr = properties.begin(); itr != properties.end(); ++itr) {
        page->insert(itr.key(), itr.value());
    }
    m_writeableCache[fileName] = AllWriteable;

    connect(page, &PageDataObject::valueChanged, this, [this, page]() {
        auto i = m_pages.indexOf(page);
        Q_EMIT dataChanged(index(i), index(i), {TitleRole, IconRole, FilesWriteableRole});
    });

    beginInsertRows(QModelIndex{}, m_pages.size(), m_pages.size());
    m_pages.append(page);
    m_pageOrder.append(fileName);
    Q_EMIT pageOrderChanged();
    endInsertRows();

    connect(page, &PageDataObject::loaded, this, [this, page] {
        auto i = m_pages.indexOf(page);
        Q_EMIT dataChanged(index(i), index(i), {TitleRole, IconRole});
    });

    return page;
}

QStringList PagesModel::pageOrder() const
{
    return m_pageOrder;
}

void PagesModel::setPageOrder(const QStringList &pageOrder)
{
    if (pageOrder != m_pageOrder) {
        m_pageOrder = pageOrder;
        Q_EMIT pageOrderChanged();
        sort();
    }
}

QStringList PagesModel::hiddenPages() const
{
    return m_hiddenPages;
}

void PagesModel::setHiddenPages(const QStringList &hiddenPages)
{
    if (hiddenPages != m_hiddenPages) {
        m_hiddenPages = hiddenPages;
        Q_EMIT hiddenPagesChanged();
        Q_EMIT dataChanged(index(0, 0), index(m_pages.count() - 1, 0), {HiddenRole});
    }
}

PageDataObject *PagesModel::importPage(const QUrl &file)
{
    const QString fileName = file.fileName();
    if (!fileName.endsWith(QLatin1String(".page"))) {
        return nullptr;
    }
    // Let addPage first figure out the file name to avoid duplicates and then reload the page after we placed the file
    auto page = addPage(fileName);
    const auto destination = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QChar('/') + page->fileName();
    QFile::copy(file.toLocalFile(), destination);
    page->resetPage();
    return page;
}

void PagesModel::removeLocalPageFiles(const QString &fileName)
{
    auto it = std::find_if(m_pages.begin(), m_pages.end(), [&](PageDataObject *page) {
        return page->fileName() == fileName;
    });
    if (it == m_pages.end()) {
        return;
    }
    if (m_writeableCache[fileName] == NotWriteable) {
        return;
    }
    PageDataObject *const page = *it;
    const QStringList files = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, page->fileName(), QStandardPaths::LocateFile);
    for (const auto &file : files) {
        if (QFileInfo(file).isWritable()) {
            QFile::remove(file);
        }
    }
    int row = it - m_pages.begin();
    if (m_writeableCache[fileName] == AllWriteable) {
        beginRemoveRows(QModelIndex(), row, row);
        m_pages.erase(it);
        m_hiddenPages.removeAll(fileName);
        m_writeableCache.remove(fileName);
        endRemoveRows();
    } else {
        page->resetPage();
        m_writeableCache[fileName] = NotWriteable;
        Q_EMIT dataChanged(index(row, 0), index(row, 0), {TitleRole, IconRole, DataRole, FilesWriteableRole});
    }
}

void PagesModel::ghnsEntryStatusChanged(const KNSCore::Entry &entry)
{
    if (!entry.isValid()) {
        return;
    }

    if (entry.status() == KNSCore::Entry::Installed) {
        const auto files = entry.installedFiles();
        for (const auto &file : files) {
            const QString fileName = QUrl::fromLocalFile(file).fileName();
            if (fileName.endsWith(QLatin1String(".page"))) {
                auto it = std::find_if(m_pages.begin(), m_pages.end(), [&](PageDataObject *page) {
                    return page->fileName() == fileName;
                });
                if (it != m_pages.end()) {
                    // User selected to overwrite the existing file in the kns dialog
                    (*it)->resetPage();
                    if (m_writeableCache[fileName] == NotWriteable) {
                        m_writeableCache[fileName] = LocalChanges;
                    }
                } else {
                    addPage(fileName.chopped(strlen(".page")));
                }
            }
        }
    } else if (entry.status() == KNSCore::Entry::Deleted) {
        const auto files = entry.uninstalledFiles();
        for (const auto &file : files) {
            const QString fileName = QUrl::fromLocalFile(file).fileName();
            if (fileName.endsWith(QLatin1String(".page"))) {
                removeLocalPageFiles(fileName);
            }
        }
    }
}

#include "moc_PagesModel.cpp"
