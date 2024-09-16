// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailmodel.h"
#include "unionimage/unionimage.h"
#include "imageengine/imagedataservice.h"
#include "unionimage/baseutils.h"
#include "imagedatamodel.h"

#include <QDebug>
#include <QIcon>
#include <QUrl>

ThumbnailModel::ThumbnailModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_viewAdapter(nullptr)
    , m_screenshotSize(256, 256)
    , m_containImages(false)
{
    setSortLocaleAware(true);
    sort(0);
    m_selectionModel = new QItemSelectionModel(this);
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &ThumbnailModel::changeSelection);
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &ThumbnailModel::selectionChanged);

    // 图片数据服务有图片加载成功，通知model刷新界面
    connect(ImageDataService::instance(), &ImageDataService::gotImage, this, &ThumbnailModel::showPreview, Qt::ConnectionType::QueuedConnection);
}

ThumbnailModel::~ThumbnailModel()
{

}

Types::ModelType ThumbnailModel::modelType() const
{
    Types::ModelType modelType = Types::Normal;
    ImageDataModel *dataModel = qobject_cast<ImageDataModel *>(sourceModel());
    if (dataModel)
        modelType = dataModel->modelType();

    return modelType;
}

void ThumbnailModel::setContainImages(bool value)
{
    m_containImages = value;
    emit containImagesChanged();
}

void ThumbnailModel::showPreview(const QString &path)
{
    int idx = indexForFilePath(path);
    if (idx != -1) {
        dataChanged(index(idx, 0, QModelIndex()), index(idx, 0, QModelIndex()));
    }
}

void ThumbnailModel::changeSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList indices = selected.indexes();
    indices.append(deselected.indexes());

    const QVector<int> roles{Roles::SelectedRole};

    for (const QModelIndex &index : indices) {
        Q_EMIT dataChanged(index, index, roles);
    }
}

QByteArray ThumbnailModel::sortRoleName() const
{
    int role = sortRole();
    return roleNames().value(role);
}

void ThumbnailModel::setSortRoleName(const QByteArray &name)
{
    if (!sourceModel()) {
        m_sortRoleName = name;
        return;
    }

    const QHash<int, QByteArray> roles = sourceModel()->roleNames();
    for (auto it = roles.begin(); it != roles.end(); it++) {
        if (it.value() == name) {
            setSortRole(it.key());
            return;
        }
    }
    qDebug() << "Sort role" << name << "not found";
}

QHash<int, QByteArray> ThumbnailModel::roleNames() const
{
    QHash<int, QByteArray> hash = sourceModel()->roleNames();
    hash.insert(Roles::BlankRole, "blank");
    hash.insert(Roles::SelectedRole, "selected");
    hash.insert(Roles::Thumbnail, "thumbnail");
    hash.insert(Roles::ReloadThumbnail, "reloadThumbnail");
    hash.insert(Roles::SourceIndex, "sourceIndex");
    hash.insert(Roles::ModelTypeRole, "modelType");
    return hash;
}

QVariant ThumbnailModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    switch (role) {

    case Roles::BlankRole: {
        return false;
    }

    case Roles::SelectedRole: {
        return m_selectionModel->isSelected(index);
    }

    case Roles::Thumbnail: {
        QString srcPath = data(index, Roles::FilePathRole).toString();
        QImage image = ImageDataService::instance()->getThumnailImageByPathRealTime(srcPath, modelType() == Types::RecentlyDeleted);
        if (!image.isNull()) {
            return image;
        } else
            return {};
    }

    case Roles::ReloadThumbnail: {
        QString srcPath = data(index, Roles::FilePathRole).toString();
        QImage image = ImageDataService::instance()->getThumnailImageByPathRealTime(srcPath, modelType() == Types::RecentlyDeleted, true);
        if (!image.isNull()) {
            return image;
        } else
            return {};
    }

    case Roles::SourceIndex: {
        return mapToSource(index).row();
    }

    case Roles::ModelTypeRole: {
        return modelType();
    }
    }

    return QSortFilterProxyModel::data(index, role);
}

bool ThumbnailModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

ThumbnailModel::Status ThumbnailModel::status() const
{
    return m_status;
}

QObject *ThumbnailModel::viewAdapter() const
{
    return m_viewAdapter;
}

void ThumbnailModel::setViewAdapter(QObject *adapter)
{
    if (m_viewAdapter != adapter) {
        ItemViewAdapter *abstractViewAdapter = dynamic_cast<ItemViewAdapter *>(adapter);

        m_viewAdapter = abstractViewAdapter;

        Q_EMIT viewAdapterChanged();
    }
}

void ThumbnailModel::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        Q_EMIT statusChanged();
    }
}

void ThumbnailModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QAbstractItemModel *oldSrcModel = QSortFilterProxyModel::sourceModel();
    if (oldSrcModel)
        disconnect(oldSrcModel, SIGNAL(modelReset()), this, SIGNAL(srcModelReseted()));

    QSortFilterProxyModel::setSourceModel(sourceModel);

    connect(sourceModel, SIGNAL(modelReset()), this, SIGNAL(srcModelReseted()));

    if (!m_sortRoleName.isEmpty()) {
        setSortRoleName(m_sortRoleName);
        m_sortRoleName.clear();
    }
}

bool ThumbnailModel::containImages()
{
    return m_containImages;
}

bool ThumbnailModel::isSelected(int row)
{
    if (row < 0) {
        return false;
    }

    return m_selectionModel->isSelected(index(row, 0));
}

void ThumbnailModel::setSelected(int indexValue)
{
    if (indexValue < 0)
        return;

    QModelIndex index = QSortFilterProxyModel::index(indexValue, 0);
    m_selectionModel->select(index, QItemSelectionModel::Select);
    emit dataChanged(index, index);
    emit selectedIndexesChanged();
}

void ThumbnailModel::toggleSelected(int indexValue)
{
    if (indexValue < 0)
        return;

    QModelIndex index = QSortFilterProxyModel::index(indexValue, 0);
    m_selectionModel->select(index, QItemSelectionModel::Toggle);
    emit dataChanged(index, index);
    emit selectedIndexesChanged();
}

void ThumbnailModel::setRangeSelected(int anchor, int to)
{
    if (anchor < 0 || to < 0) {
        return;
    }

    QItemSelection selection(index(anchor, 0), index(to, 0));
    m_selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);

    emit selectedIndexesChanged();
}

void ThumbnailModel::updateSelection(const QVariantList &rows, bool toggle)
{
    QItemSelection newSelection;

    QList<int> oldSelecteds = selectedIndexes();

    int iRow = -1;

    for (const QVariant &row : rows) {
        iRow = row.toInt();

        if (iRow < 0) {
            return;
        }

        const QModelIndex &idx = index(iRow, 0);
        newSelection.select(idx, idx);
    }

    if (toggle) {
        QItemSelection pinnedSelection = m_pinnedSelection;
        pinnedSelection.merge(newSelection, QItemSelectionModel::Toggle);
        m_selectionModel->select(pinnedSelection, QItemSelectionModel::ClearAndSelect);
    } else {
        m_selectionModel->select(newSelection, QItemSelectionModel::ClearAndSelect);
    }

    // 仅选择项有改变，才向外部通知选中内容改变
    if (oldSelecteds != selectedIndexes())
        emit selectedIndexesChanged();
}

void ThumbnailModel::clearSelection()
{
    if (m_selectionModel->hasSelection()) {
        QModelIndexList selectedIndex = m_selectionModel->selectedIndexes();
        m_selectionModel->clear();
//        for (auto indexValue : selectedIndex) {
//            emit dataChanged(indexValue, indexValue);
//        }
    }
    emit selectedIndexesChanged();
}

void ThumbnailModel::pinSelection()
{
    m_pinnedSelection = m_selectionModel->selection();
}

void ThumbnailModel::unpinSelection()
{
    m_pinnedSelection = QItemSelection();
}

void ThumbnailModel::selectAll()
{
    setRangeSelected(0, rowCount() - 1);
}

int ThumbnailModel::proxyIndex(const int &indexValue)
{
    if (sourceModel()) {
        return mapFromSource(sourceModel()->index(indexValue, 0, QModelIndex())).row();
    }
    return -1;
}

int ThumbnailModel::sourceIndex(const int &indexValue)
{
    return mapToSource(index(indexValue, 0, QModelIndex())).row();
}

QJsonArray ThumbnailModel::allUrls()
{
    QJsonArray arr;
    for (int row = 0; row < rowCount(); row++)
        arr.append(QJsonValue(data(index(row, 0), Roles::UrlRole).toString()));

    return arr;
}

QJsonArray ThumbnailModel::allPaths()
{
    QJsonArray arr;
    for (int row = 0; row < rowCount(); row++)
        arr.append(QJsonValue(data(index(row, 0), Roles::FilePathRole).toString()));

    return arr;
}

QJsonArray ThumbnailModel::selectedUrls()
{
    QJsonArray arr;

    for (auto index : m_selectionModel->selectedIndexes())
        arr.push_back(QJsonValue(data(index, Roles::UrlRole).toString()));

    return arr;
}

QJsonArray ThumbnailModel::selectedPaths()
{
    QJsonArray arr;

    for (auto index : m_selectionModel->selectedIndexes())
        arr.push_back(QJsonValue(data(index, Roles::FilePathRole).toString()));

    return arr;
}

QList<int> ThumbnailModel::selectedIndexes()
{
    QList<int> selects;
    for (auto index : m_selectionModel->selectedIndexes()) {
        selects.push_back(index.row());
    }

    return selects;
}

int ThumbnailModel::indexForUrl(const QString &url)
{
    QModelIndexList indexList;
    for (int row = 0; row < rowCount(); row++) {
        indexList.append(index(row, 0, QModelIndex()));
    }
    for (auto index : indexList) {
        if (url == data(index, Roles::UrlRole).toString()) {
            return index.row();
        }
    }
    return -1;
}

QList<int> ThumbnailModel::indexesForUrls(const QStringList &urls)
{
    QList<int> indexes;
    for (int row = 0; row < rowCount(); row++) {
        if (urls.indexOf(data(index(row, 0, QModelIndex()), Roles::UrlRole).toString()) != -1)
            indexes.push_back(row);
    }

    return indexes;
}

int ThumbnailModel::indexForFilePath(const QString &filePath)
{
    QModelIndexList indexList;
    for (int row = 0; row < rowCount(); row++) {
        indexList.append(index(row, 0, QModelIndex()));
    }
    if (modelType() == Types::RecentlyDeleted) {
        for (auto index : indexList) {
            QString path = data(index, Roles::FilePathRole).toString();
            if (filePath == Libutils::base::getDeleteFullPath(Libutils::base::hashByString(path), DBImgInfo::getFileNameFromFilePath(path))) {
                return index.row();
            }
        }
    } else {
        for (auto index : indexList) {
            if (filePath == data(index, Roles::FilePathRole).toString()) {
                return index.row();
            }
        }
    }

    return -1;
}

QVariant ThumbnailModel::data(int idx, const QString &role)
{
    QHash<int, QByteArray> hash = roleNames();
    int roleType = -1;
    QHash<int, QByteArray>::const_iterator i = hash.constBegin();
    while (i != hash.constEnd()) {
        int nType = i.key();
        if (i.value() == role) {
            roleType = nType;
            break;
        }
        i++;
    }

    if (roleType != -1)
        return data(index(idx, 0, QModelIndex()), roleType);

    return QVariant();
}

void ThumbnailModel::refresh(int type)
{
    ImageDataModel *dataModel = qobject_cast<ImageDataModel *>(sourceModel());
    if (dataModel)
        dataModel->loadData(static_cast<Types::ItemType>(type));
}

void ThumbnailModel::selectUrls(const QStringList &urls)
{
    QItemSelection newSelection;

    QList<int> indexes = indexesForUrls(urls);
    for (auto i : indexes) {
        const QModelIndex &idx = index(i, 0);
        newSelection.select(idx, idx);
    }

    m_selectionModel->select(newSelection, QItemSelectionModel::ClearAndSelect);
}

DBImgInfo ThumbnailModel::indexForData(const QModelIndex &index) const
{
    if (!index.isValid())
        return DBImgInfo();

    return index.data().value<DBImgInfo>();
}
