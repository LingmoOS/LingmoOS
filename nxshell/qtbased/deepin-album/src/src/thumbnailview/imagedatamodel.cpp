// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagedatamodel.h"
#include "roles.h"
#include "dbmanager/dbmanager.h"
#include "albumControl.h"

#include <QUrl>

ImageDataModel::ImageDataModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_modelType(Types::ModelType::Normal)
    , m_albumID(-1)
    , m_keyWord("")
    , m_dayToken("")
{

}

QHash<int, QByteArray> ImageDataModel::roleNames() const
{
    auto hash = QAbstractItemModel::roleNames();
    // the url role returns the url of the cover image of the collection
    hash.insert(Roles::ItemTypeRole, "itemType");

    hash.insert(Roles::FileNameRole, "fileName");
    hash.insert(Roles::UrlRole, "url");
    hash.insert(Roles::FilePathRole, "filePath");
    hash.insert(Roles::PathHashRole, "pathHash");
    hash.insert(Roles::RemainDaysRole, "remainDays");

    return hash;
}

QVariant ImageDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const DBImgInfo &info = m_infoList.at(index.row());

    switch (role) {
    case Qt::DisplayRole: {
        return QVariant::fromValue(info);
    }

    case Roles::FileNameRole: {
        return QUrl::fromLocalFile(info.filePath).fileName();
    }

    case Roles::UrlRole: {
        QString filePath = "";
        if (Types::RecentlyDeleted == m_modelType) {
            filePath = AlbumControl::instance()->getDeleteFullPath(LibUnionImage_NameSpace::hashByString(info.filePath), DBImgInfo::getFileNameFromFilePath(info.filePath));
        } else
            filePath = info.filePath;
        return QUrl::fromLocalFile(filePath).toString();
    }

    case Roles::FilePathRole: {
        return info.filePath;
    }

    case Roles::PathHashRole: {
        return info.pathHash;
    }

    case Roles::RemainDaysRole: {
        return info.remainDays;
    }

    case Roles::ItemTypeRole: {
        if (info.itemType == ItemTypePic) {
            return "picture";
        } else if (info.itemType == ItemTypeVideo) {
            return "video";
        } else {
            return "other";
        }
    }
    }

    return {};
}

int ImageDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_infoList.size();
}

Types::ModelType ImageDataModel::modelType() const
{
    return m_modelType;
}

void ImageDataModel::setModelType(Types::ModelType modelType)
{
    beginResetModel();
    m_modelType = modelType;
    endResetModel();

    emit modelTypeChanged();
}

int ImageDataModel::albumId() const
{
    return m_albumID;
}

void ImageDataModel::setAlbumId(int albumId)
{
    if (albumId != m_albumID)
        emit albumIdChanged();

    m_albumID = albumId;
}

QString ImageDataModel::keyWord()
{
    return m_keyWord;
}

void ImageDataModel::setKeyWord(QString keyWord)
{
    if (keyWord != m_keyWord)
        emit keyWordChanged();

    m_keyWord = keyWord;
}

QString ImageDataModel::devicePath()
{
    return m_devicePath;
}

void ImageDataModel::setDevicePath(QString devicePath)
{
    if (m_devicePath != devicePath)
        emit devicePathChanged();

    m_devicePath = devicePath;
}

QString ImageDataModel::dayToken()
{
    return m_dayToken;
}

void ImageDataModel::setDayToken(QString dayToken)
{
    if (m_dayToken != dayToken)
        emit dayTokenChanged();

    m_dayToken = dayToken;
}

QString ImageDataModel::importTitle()
{
    return m_importTitle;
}

void ImageDataModel::setImportTitle(QString importTitle)
{
    if (m_importTitle != importTitle)
        emit importTitleChanged();

    m_importTitle = importTitle;
}

DBImgInfo ImageDataModel::dataForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return DBImgInfo();

    return m_infoList.at(index.row());
}

void ImageDataModel::loadData(Types::ItemType type)
{
    QElapsedTimer time;
    time.start();
    ItemType itemType = ItemTypeNull;
    if (type == Types::All)
        itemType = ItemTypeNull;
    else if (type == Types::Picture)
        itemType = ItemTypePic;
    else if (type == Types::Video)
        itemType = ItemTypeVideo;

    beginResetModel();
    if (m_modelType == Types::AllCollection)
        m_infoList = DBManager::instance()->getAllInfosSort(itemType);
    else if (m_modelType == Types::CustomAlbum)
        m_infoList = DBManager::instance()->getInfosByAlbum(m_albumID, false, itemType);
    else if (m_modelType == Types::RecentlyDeleted) {
        m_infoList = AlbumControl::instance()->getTrashInfos2(itemType);
    } else if (m_modelType == Types::Device) {
        m_infoList = AlbumControl::instance()->getDeviceAlbumInfos2(m_devicePath, itemType);
    } else if (m_modelType == Types::SearchResult) {
        m_infoList = AlbumControl::instance()->searchPicFromAlbum2(m_albumID, m_keyWord, false);
    } else if (m_modelType == Types::DayCollecttion) {
        m_infoList = DBManager::instance()->getInfosByDay(m_dayToken);
    } else if (m_modelType == Types::HaveImported) {
        m_infoList = DBManager::instance()->getInfosByImportTimeline(QDateTime::fromString(m_importTitle, "yyyy/MM/dd hh:mm"), itemType);
    }
    endResetModel();
    qDebug() << QString("loadData modelType:[%1] cost [%2]ms..").arg(m_modelType).arg(time.elapsed());
}
