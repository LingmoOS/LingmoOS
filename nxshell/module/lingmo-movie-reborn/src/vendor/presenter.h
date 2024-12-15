// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRESENTER_H
#define PRESENTER_H

#include <QObject>
#include <MprisPlayer>
#include "mainwindow.h"
#include "platform/platform_mainwindow.h"
#include "mpv_proxy.h"
#include "player_engine.h"
#include "dmr_settings.h"
#include "platform/platform_notification_widget.h"
#include "platform/platform_toolbox_proxy.h"
#include "playlist_model.h"

using namespace dmr;

class Presenter : public QObject
{
    Q_OBJECT
public:
    explicit Presenter(MainWindow* mw, QObject *parent = nullptr);
    explicit Presenter(Platform_MainWindow* mw, QObject *parent = nullptr);

    void initMpris(MprisPlayer *mprisPlayer);

signals:

public slots:
    void slotplay();
    void slotpause();
    void slotplaynext();
    void slotplayprev();
    void slotvolumeRequested(double volume);
    void slotopenUrlRequested(const QUrl url);
    void slotstateChanged();
    void slotloopStatusRequested(Mpris::LoopStatus loopStatus);
    void slotplayModeChanged(PlaylistModel::PlayMode pm);
    void slotvolumeChanged();
    void slotseek(qlonglong Offset);
    void slotstop();
private:
    MainWindow* _mw = nullptr;
    Platform_MainWindow* _platform_mw = nullptr;
    MprisPlayer* m_mprisplayer=nullptr;
};

#endif // PRESENTER_H
