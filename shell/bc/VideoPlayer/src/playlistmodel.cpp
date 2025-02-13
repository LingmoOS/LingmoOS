/*
 * SPDX-FileCopyrightText: 2021 Reion Wong <reion@cuteos.com>
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "playlistmodel.h"

#include <QStandardPaths>
#include <QCollator>
#include <QDirIterator>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QUrl>

PlayListModel::PlayListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_settings("lingmoos", "lingmo-videoplayer")
{
    // Reion: init list.
    QStringList list = m_settings.value("List").toString().split(',');
    int idx = 0;

    for (const QString &path : list) {
        if (!QFile(path).exists())
            continue;

        auto video = new PlayListItem(path, idx , this);
        m_playList.append(video);
        ++idx;
    }

    if (!m_playList.isEmpty())
        setPlayingVideo(0);

//    m_config = KSharedConfig::openConfig(Global::instance()->appConfigFilePath());
//    connect(this, &PlayListModel::videoAdded,
//            Worker::instance(), &Worker::getMetaData);

//    connect(Worker::instance(), &Worker::metaDataReady, this, [ = ](int i, KFileMetaData::PropertyMap metaData) {
//        auto duration = metaData[KFileMetaData::Property::Duration].toInt();
//        auto title = metaData[KFileMetaData::Property::Title].toString();

//        m_playList[i]->setDuration(Application::formatTime(duration));
//        m_playList[i]->setMediaTitle(title);

//        emit dataChanged(index(i, 0), index(i, 0));

//    });
}

int PlayListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_playList.size();
}

QVariant PlayListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_playList.empty())
        return QVariant();

    auto playListItem = m_playList.at(index.row());
    switch (role) {
    case NameRole:
        return QVariant(playListItem->fileName());
    case TitleRole:
        return playListItem->mediaTitle().isEmpty()
                ? QVariant(playListItem->fileName())
                : QVariant(playListItem->mediaTitle());
    case PathRole:
        return QVariant(playListItem->filePath());
    case DurationRole:
        return QVariant(playListItem->duration());
    case PlayingRole:
        return QVariant(playListItem->isPlaying());
    case FolderPathRole:
        return QVariant(playListItem->folderPath());
    }

    return QVariant();
}

QHash<int, QByteArray> PlayListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[TitleRole] = "title";
    roles[PathRole] = "path";
    roles[FolderPathRole] = "folderPath";
    roles[DurationRole] = "duration";
    roles[PlayingRole] = "isPlaying";
    return roles;
}

void PlayListModel::getVideos(QString path)
{
    clear();
    path = QUrl(path).toLocalFile().isEmpty() ? path : QUrl(path).toLocalFile();
    QFileInfo pathInfo(path);
    QStringList videoFiles;
    if (pathInfo.exists() && pathInfo.isFile()) {
        QDirIterator it(pathInfo.absolutePath(), QDir::Files, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            QString file = it.next();
            QFileInfo fileInfo(file);
            QMimeDatabase db;
            QMimeType type = db.mimeTypeForFile(file);
            if (fileInfo.exists() && (type.name().startsWith("video/") || type.name().startsWith("audio/"))) {
                videoFiles.append(fileInfo.absoluteFilePath());
            }
        }
    }
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(videoFiles.begin(), videoFiles.end(), collator);

    beginInsertRows(QModelIndex(), 0, videoFiles.count() - 1);

    for (int i = 0; i < videoFiles.count(); ++i) {
        auto video = new PlayListItem(videoFiles.at(i), i, this);
        m_playList.append(video);
        if (path == videoFiles.at(i)) {
            setPlayingVideo(i);
        }
        emit videoAdded(i, video->filePath());
    }

    endInsertRows();
}

Playlist PlayListModel::items() const
{
    return m_playList;
}

Playlist PlayListModel::getPlayList() const
{
    return m_playList;
}

void PlayListModel::setPlayList(const Playlist &playList)
{
    beginInsertRows(QModelIndex(), 0, playList.size() - 1);
    m_playList = playList;
    endInsertRows();
}

int PlayListModel::getPlayingVideo() const
{
    return m_playingVideo;
}

void PlayListModel::clear()
{
    m_playingVideo = 0;
    qDeleteAll(m_playList);
    beginResetModel();
    m_playList.clear();
    endResetModel();
}

void PlayListModel::save()
{
    QString value;
    for (int i = 0; i < m_playList.size(); ++i) {
        const QString path = m_playList.at(i)->filePath();

        if (value.isEmpty())
            value.append(path);
        else
            value.append(',' + path);
    }

    m_settings.setValue("List", value);
}

void PlayListModel::addVideo(const QString &path)
{
    QString newPath = QUrl(path).toLocalFile().isEmpty() ? path : QUrl(path).toLocalFile();

    // Skip
    for (int i = 0; i < m_playList.size(); ++i) {
        if (m_playList.at(i)->filePath() == newPath) {
            return;
        }
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    auto video = new PlayListItem(newPath, m_playList.size() - 1, this);
    m_playList.append(video);
    setPlayingVideo(m_playList.size() - 1);
    endInsertRows();

    save();
}

void PlayListModel::remove(int i)
{
    if (i < 0 || i > m_playList.size())
        return;

    setPlayingVideo(i);

    beginRemoveRows(QModelIndex(), i, i);
    PlayListItem *item = m_playList.at(i);
    m_playList.removeAt(i);
    item->deleteLater();
    endRemoveRows();
    save();
}

QString PlayListModel::getPath(int i)
{
    // when restoring a youtube playlist
    // ensure the requested path is valid
    if (m_playList.isEmpty()) {
        return QString();
    }
    if (m_playList.size() <= i) {
        return m_playList[0]->filePath();
    }
    return m_playList[i]->filePath();
}

PlayListItem *PlayListModel::getItem(int i)
{
    if (m_playList.size() <= i) {
        return m_playList[0];
    }
    return m_playList[i];
}

void PlayListModel::setPlayingVideo(int playingVideo)
{
    // unset current playing video
    m_playList[m_playingVideo]->setIsPlaying(false);
    emit dataChanged(index(m_playingVideo, 0), index(m_playingVideo, 0));

    // set new playing video
    m_playList[playingVideo]->setIsPlaying(true);
    emit dataChanged(index(playingVideo, 0), index(playingVideo, 0));

    m_playingVideo = playingVideo;
    emit playingVideoChanged();
}
