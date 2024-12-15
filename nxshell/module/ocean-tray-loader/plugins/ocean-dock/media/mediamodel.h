// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include "dbusmediaplayer2.h"
#include "mediacontroller.h"

#include <QPixmap>
#include <QObject>
#include <QPointer>

#include <DSingleton>

class MediaModel : public QObject, public Dtk::Core::DSingleton<MediaModel>
{
    friend class Dtk::Core::DSingleton<MediaModel>;

    Q_OBJECT
public:

    struct MediaInfo {
        QPixmap pixmap;
        QString artist;
        QString title;
    };
    const QString &path() { return m_path; };
    void setPath(const QString &path);

    bool playState() { return m_playStatus; };
    void setPlayState(bool state);

    bool controlState() { return m_controlState; };
    void setControlState(bool state) { m_controlState = state; }
    void setMediaInfo(const MediaInfo &info);
    const MediaInfo &mediaInfo() { return m_info; }

public Q_SLOTS:
    void onCanControlChanged(bool canControl);

Q_SIGNALS:
    void mediaAcquired() const;
    void mediaChanged() const;
    void mediaLosted() const;

    void controlStateChanged(bool state);
    void playStateChanged(bool state);
    void mediaPathChanged(const QString &path);
    void mediaInfoChanged(const MediaInfo &info);

private:
    MediaModel();
    ~MediaModel();

private:
    QString     m_path;
    bool        m_playStatus;
    bool        m_controlState;
    MediaInfo   m_info;
};

#endif
