// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mediamodel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MediaModel::MediaModel()
    : m_path(QString())
    , m_playStatus(false)
    , m_info(MediaInfo())
{

}

MediaModel::~MediaModel()
{

}

void MediaModel::setMediaInfo(const MediaInfo &info)
{
    if (m_info.pixmap.toImage() != info.pixmap.toImage() || m_info.artist != info.artist || m_info.title != info.title) {
        m_info = info;
        Q_EMIT mediaInfoChanged(info);
    }
}

 void MediaModel::setPath(const QString &path)
 {
    if (m_path != path) {
        m_path = path;
        Q_EMIT mediaPathChanged(path);
    }
 }

void MediaModel::setPlayState(bool state)
{
    if (m_playStatus != state) {
        m_playStatus = state;
        Q_EMIT playStateChanged(state);
    }
}

void MediaModel::onCanControlChanged(bool canControl)
{
    if (m_controlState != canControl) {
        m_controlState = canControl;
        Q_EMIT controlStateChanged(canControl);
    }
}
