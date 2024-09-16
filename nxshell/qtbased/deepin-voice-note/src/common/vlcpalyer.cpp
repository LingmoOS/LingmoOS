// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vlcpalyer.h"

#include <vlc/vlc.h>
#include <vlc/libvlc_media.h>

#include <DApplication>

DWIDGET_USE_NAMESPACE
/**
 * @brief VlcPalyer::VlcPalyer
 * @param parent
 */
VlcPalyer::VlcPalyer(QObject *parent)
    : QObject(parent)
{
#ifdef MPV_PLAYENGINE
    init();
#endif
}

/**
 * @brief VlcPalyer::~VlcPalyer
 */
VlcPalyer::~VlcPalyer()
{
    deinit();
}

/**
 * @brief VlcPalyer::init
 */
void VlcPalyer::init()
{
#ifndef MPV_PLAYENGINE
    qInfo() << "The current play engine is vlc";
    if (m_vlcInst == nullptr) {
        m_vlcInst = libvlc_new(0, nullptr);
        libvlc_set_user_agent(m_vlcInst, DApplication::translate("AppMain", "Voice Notes").toUtf8().constData(), "");
        libvlc_set_app_id(m_vlcInst, "", "", "deepin-voice-note");
    }
    if (m_vlcPlayer == nullptr) {
        m_vlcPlayer = libvlc_media_player_new(m_vlcInst);
        libvlc_media_player_set_role(m_vlcPlayer, libvlc_role_None);
        attachEvent();
    }
#else
    qInfo() << "The current play engine is mpv";
    setlocale(LC_NUMERIC, "C");
    player = new dmr::PlayerEngine(nullptr);
    player->hide();
    player->getplaylist()->setPlayMode(PlaylistModel::PlayMode::SinglePlay);
    connect(player, &dmr::PlayerEngine::elapsedChanged, this, &VlcPalyer::onGetCurrentPosition);
    connect(player, &dmr::PlayerEngine::stateChanged, this, &VlcPalyer::onGetState);
    connect(player, &dmr::PlayerEngine::fileLoaded, this, &VlcPalyer::onGetduration);
#endif
}

/**
 * @brief VlcPalyer::deinit
 */
void VlcPalyer::deinit()
{
#ifndef MPV_PLAYENGINE
    if (m_vlcPlayer) {
        detachEvent();
        libvlc_media_t *media = libvlc_media_player_get_media(m_vlcPlayer);
        if (media) {
            libvlc_media_release(media);
        }
        libvlc_media_player_release(m_vlcPlayer);
        m_vlcPlayer = nullptr;
    }
    if (m_vlcInst) {
        libvlc_release(m_vlcInst);
        m_vlcInst = nullptr;
    }
#else
    if (player != nullptr) {
        player->hide();
        disconnect(player, &dmr::PlayerEngine::elapsedChanged, this, &VlcPalyer::onGetCurrentPosition);
        disconnect(player, &dmr::PlayerEngine::stateChanged, this, &VlcPalyer::onGetState);
        disconnect(player, &dmr::PlayerEngine::fileLoaded, this, &VlcPalyer::onGetduration);
        player->deleteLater();
    }
#endif
}

#ifndef MPV_PLAYENGINE
/**
 * @brief VlcPalyer::attachEvent
 */
void VlcPalyer::attachEvent()
{
    libvlc_event_manager_t *vlc_evt_man = libvlc_media_player_event_manager(m_vlcPlayer);
    if (vlc_evt_man) {
        libvlc_event_attach(vlc_evt_man, libvlc_MediaPlayerEndReached, handleEvent, this);
        libvlc_event_attach(vlc_evt_man, libvlc_MediaPlayerTimeChanged, handleEvent, this);
        libvlc_event_attach(vlc_evt_man, libvlc_MediaPlayerLengthChanged, handleEvent, this);
    }
}

/**
 * @brief VlcPalyer::detachEvent
 */
void VlcPalyer::detachEvent()
{
    libvlc_event_manager_t *vlc_evt_man = libvlc_media_player_event_manager(m_vlcPlayer);
    if (vlc_evt_man) {
        libvlc_event_detach(vlc_evt_man, libvlc_MediaPlayerEndReached, handleEvent, this);
        libvlc_event_detach(vlc_evt_man, libvlc_MediaPlayerTimeChanged, handleEvent, this);
        libvlc_event_detach(vlc_evt_man, libvlc_MediaPlayerLengthChanged, handleEvent, this);
    }
}
#endif

/**
 * @brief VlcPalyer::setFilePath
 * @param path 文件路径
 */
void VlcPalyer::setFilePath(QString path)
{
    qDebug() << "Current playback file: " << path;
#ifndef MPV_PLAYENGINE
    init();
    libvlc_media_t *media = libvlc_media_new_path(m_vlcInst, path.toLatin1().constData());
    if (media) {
        libvlc_media_player_set_media(m_vlcPlayer, media);
        libvlc_media_release(media);
    }
#else
    if (player->isPlayableFile(path)){
       videoUrl = QUrl::fromLocalFile(path);
    }
#endif
}

/**
 * @brief VlcPalyer::play
 */
void VlcPalyer::play()
{
    qInfo() << "Start playing audio";
#ifndef MPV_PLAYENGINE
    if (m_vlcPlayer) {
        libvlc_media_player_play(m_vlcPlayer);
    }
#else
    qInfo() << "Current play engine status: " << player->state();
    if(player->state() == PlayerEngine::CoreState::Paused && !m_isChangePlayFile){
        qInfo() << "Pause start: " << videoUrl;
        player->pauseResume();
    }else{
        qInfo() << "Play new audio: " << videoUrl;
        player->addPlayFile(videoUrl);
        player->playByName(videoUrl);
    }
#endif
}

/**
 * @brief VlcPalyer::pause
 */
void VlcPalyer::pause()
{
    qInfo() << "Pause play";
#ifndef MPV_PLAYENGINE
    if (m_vlcPlayer) {
        libvlc_media_player_pause(m_vlcPlayer);
    }
#else
    player->pauseResume();
#endif
}

/**
 * @brief VlcPalyer::stop
 */
void VlcPalyer::stop()
{
    qInfo() << "Stop playing";
#ifndef MPV_PLAYENGINE
    if (m_vlcPlayer) {
        libvlc_media_player_stop(m_vlcPlayer);
    }
#else
    player->stop();
#endif

}
#ifdef MPV_PLAYENGINE
/**
 * @brief VlcPalyer::onGetCurrentPosition
 */
void VlcPalyer::onGetCurrentPosition()
{
//    qInfo() << "The current play position has changed!" << player->engine().elapsed();
     emit positionChanged(player->elapsed());
}

/**
 * @brief VlcPalyer::onGetState
 */
void VlcPalyer::onGetState()
{
    qInfo() << "Change the current audio playback status!(status: " << player->state() << ")";
    if(player->state() == PlayerEngine::CoreState::Idle && !m_isChangePlayFile)
    {
        qInfo() << "The current audio playback is complete! (status: " << player->state() << ")";
        emit playEnd();
    }else if(player->state() == PlayerEngine::CoreState::Playing && m_isChangePlayFile){
        m_isChangePlayFile = false;
    }
}
void VlcPalyer::onGetduration()
{
    qInfo() << "Total audio duration changed! (duration: " << player->duration() << ")";
    emit durationChanged(player->duration());
}
#endif

#ifndef MPV_PLAYENGINE

/**
 * @brief VlcPalyer::handleEvent
 * @param event
 * @param data
 */
void VlcPalyer::handleEvent(const libvlc_event_t *event, void *data)
{
    VlcPalyer *userData = static_cast<VlcPalyer *>(data);
    switch (event->type) {
    case libvlc_MediaPlayerEndReached:
        qInfo() << "The current audio playback is complete!";
        emit userData->playEnd();
        break;
    case libvlc_MediaPlayerTimeChanged:
        qInfo() << "The current play position has changed!";
        emit userData->positionChanged(event->u.media_player_time_changed.new_time);
        break;
    case libvlc_MediaPlayerLengthChanged:
        qInfo() << "Total audio duration changed!";
        emit userData->durationChanged(event->u.media_duration_changed.new_duration);
        break;
    default:
        break;
    }
}
#endif

/**
 * @brief VlcPalyer::setPosition
 * @param pos
 */
void VlcPalyer::setPosition(qint64 pos)
{
    qInfo() << "Set the current audio playback position.(pos: " << pos << ")";
#ifndef MPV_PLAYENGINE
    if (m_vlcPlayer) {
        libvlc_media_player_set_time(m_vlcPlayer, pos);
    }
#else
    player->seekAbsolute(pos);
#endif
}

/**
 * @brief VlcPalyer::getState
 * @return 状态
 */
VlcPalyer::VlcState VlcPalyer::getState()
{
    VlcPalyer::VlcState state = None;
#ifndef MPV_PLAYENGINE
    if (m_vlcPlayer) {
        state = static_cast<VlcPalyer::VlcState>(libvlc_media_player_get_state(m_vlcPlayer));
    }
    qInfo() << "Gets the current audio status.(status: " << state << ")" ;
#else
    qInfo() << "Gets the current audio status.(status: " << player->state() << ")" ;
    if(player->state() == PlayerEngine::CoreState::Idle){
        state = VlcPalyer::VlcState::Stopped;
    }else if(player->state() == PlayerEngine::CoreState::Playing){
        state = VlcPalyer::VlcState::Playing;
    }else{
        state = VlcPalyer::VlcState::Paused;
    }
#endif
    return state;
}

void VlcPalyer::setChangePlayFile(bool flag)
{
    m_isChangePlayFile = flag;
}
