// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include <iostream>
#include "player_engine.h"
#include "playlist_model.h"
#include "movie_configuration.h"
#include "online_sub.h"
#include "mpv_proxy.h"
#include "compositing_manager.h"
#include "dguiapplicationhelper.h"
#include "filefilter.h"
#include "qtplayer_proxy.h"
#include "eventlogutils.h"

#include <QPainterPath>

#ifndef _LIBDMR_
#include "dmr_settings.h"
#endif

#include "drecentmanager.h"
DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace dmr {

const QStringList PlayerEngine::audio_filetypes = {"*.mp3", "*.wav", "*.wma", "*.m4a", "*.aac", "*.ac3", "*.ape", "*.flac", "*.ra", "*.mka", "*.dts", "*.opus", "*.amr"};
const QStringList PlayerEngine::video_filetypes = {
    "*.3g2", "*.3ga", "*.3gp", "*.3gp2", "*.3gpp", "*.amv", "*.asf", "*.asx", "*.avf", "*.avi", "*.bdm", "*.bdmv", "*.bik", "*.clpi", "*.cpi",
    "*.dat", "*.divx", "*.drc", "*.dv", "*.dvr-ms", "*.f4v", "*.flv", "*.gvi", "*.gxf", "*.hdmov", "*.hlv", "*.iso", "*.letv", "*.lrv", "*.m1v", "*.m2p", "*.m2t", "*.m2ts",
    "*.m2v", "*.m3u", "*.m3u8", "*.m4v", "*.mkv", "*.moov", "*.mov", "*.mov", "*.mp2", "*.mp2v", "*.mp4", "*.mp4v", "*.mpe", "*.mpeg", "*.mpeg1", "*.mpeg2", "*.mpeg4", "*.mpg",
    "*.mpl", "*.mpls", "*.mpv", "*.mpv2", "*.mqv", "*.mts", "*.mts", "*.mtv", "*.mxf", "*.mxg", "*.nsv", "*.nuv", "*.ogg", "*.ogm", "*.ogv", "*.ogx", "*.ps", "*.qt", "*.qtvr",
    "*.ram", "*.rec", "*.rm", "*.rm", "*.rmj", "*.rmm", "*.rms", "*.rmvb", "*.rmx", "*.rp", "*.rpl", "*.rv", "*.rvx", "*.thp", "*.tod", "*.tp", "*.trp", "*.ts", "*.tts", "*.txd",
    "*.vcd", "*.vdr", "*.vob", "*.vp8", "*.vro", "*.webm", "*.wm", "*.wmv", "*.wtv", "*.xesc", "*.xspf", "*.ogg",
};

const QStringList PlayerEngine::subtitle_suffixs = {"ass", "sub", "srt", "aqt", "jss", "gsub", "ssf", "ssa", "smi", "usf", "idx"};

PlayerEngine::PlayerEngine(QWidget *parent)
    : QWidget(parent)
{
    m_bAudio = false;
    m_stopRunningThread = false;
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    if(CompositingManager::isMpvExists()){
        _current = new MpvProxy(this);
    } else {
        _current = new QtPlayerProxy(this);
    }
    if (_current) {
        connect(_current, &Backend::stateChanged, this, &PlayerEngine::onBackendStateChanged);
        connect(_current, &Backend::tracksChanged, this, &PlayerEngine::tracksChanged);
        connect(_current, &Backend::elapsedChanged, this, &PlayerEngine::elapsedChanged);
        connect(_current, &Backend::fileLoaded, this, &PlayerEngine::fileLoaded);
        connect(_current, &Backend::muteChanged, this, &PlayerEngine::muteChanged);
        connect(_current, &Backend::volumeChanged, this, &PlayerEngine::volumeChanged);
        connect(_current, &Backend::sidChanged, this, &PlayerEngine::sidChanged);
        connect(_current, &Backend::aidChanged, this, &PlayerEngine::aidChanged);
        connect(_current, &Backend::videoSizeChanged, this, &PlayerEngine::videoSizeChanged);
        connect(_current, &Backend::notifyScreenshot, this, &PlayerEngine::notifyScreenshot);
        connect(_current, &Backend::mpvErrorLogsChanged, this, &PlayerEngine::mpvErrorLogsChanged);
        connect(_current, &Backend::mpvWarningLogsChanged, this, &PlayerEngine::mpvWarningLogsChanged);
        connect(_current, &Backend::urlpause, this, &PlayerEngine::urlpause);
        connect(_current, &Backend::sigMediaError, this, &PlayerEngine::sigMediaError);
        l->addWidget(_current);
    }

    connect(&_networkConfigMng, &QNetworkConfigurationManager::onlineStateChanged, this, &PlayerEngine::onlineStateChanged);

    setLayout(l);


#ifndef _LIBDMR_
    connect(&Settings::get(), &Settings::subtitleChanged, this, &PlayerEngine::updateSubStyles);
#endif

    connect(&OnlineSubtitle::get(), &OnlineSubtitle::subtitlesDownloadedFor,
            this, &PlayerEngine::onSubtitlesDownloaded);

    _playlist = new PlaylistModel(this);
    connect(_playlist, &PlaylistModel::asyncAppendFinished, this,
            &PlayerEngine::onPlaylistAsyncAppendFinished, Qt::DirectConnection);
    connect(_playlist, &PlaylistModel::updateDuration, this, &PlayerEngine::updateDuration);
}

PlayerEngine::~PlayerEngine()
{
    m_stopRunningThread = true;
    FileFilter::instance()->stopThread();
    if (_current) {
        disconnect(_current, nullptr, nullptr, nullptr);
        delete _current;
        _current = nullptr;
    }
    
    if (_playlist) {
        disconnect(_playlist, nullptr, nullptr, nullptr);
        delete _playlist;
        _playlist = nullptr;
    }
    qInfo() << __func__;
}

bool PlayerEngine::isPlayableFile(const QUrl &url)
{
    if (FileFilter::instance()->isMediaFile(url)) {
        return true;
    } else {    // 网络文件不提示
        if(url.isLocalFile()) {
            emit sigInvalidFile(QFileInfo(url.toLocalFile()).fileName());
        }
        return false;
    }
}

static QStringList suffixes;

bool PlayerEngine::isPlayableFile(const QString &name)
{
    QUrl url = FileFilter::instance()->fileTransfer(name);

    if (FileFilter::instance()->isMediaFile(url))
    {
        return true;
    }

    if (url.isLocalFile()) {   // 网络文件不提示
        emit sigInvalidFile(QFileInfo(url.toLocalFile()).fileName());
        return false;
    }
    return  false;
}

bool PlayerEngine::isAudioFile(const QString &name)
{
    QUrl url = FileFilter::instance()->fileTransfer(name);

    return FileFilter::instance()->isAudio(url);
}

bool PlayerEngine::isSubtitle(const QString &name)
{
    QUrl url = FileFilter::instance()->fileTransfer(name);

    return FileFilter::instance()->isSubtitle(url);
}

void PlayerEngine::updateSubStyles()
{
#ifndef _LIBDMR_
    QPointer<DSettingsOption> pFontOpt = Settings::get().settings()->option("subtitle.font.family");
    QPointer<DSettingsOption> pSizeOpt = Settings::get().settings()->option("subtitle.font.size");
    if(!pFontOpt || !pSizeOpt)
    {
        return;
    }

    int fontId = pFontOpt->value().toInt();
    int size = pSizeOpt->value().toInt();
    QString font = pFontOpt->data("items").toStringList()[fontId];

    if (_state != CoreState::Idle) {
        if (_playlist->current() < 0) return;

        auto vh = videoSize().height();
        if (vh <= 0) {
            vh = _playlist->currentInfo().mi.height;
        }
        double scale = vh / 720.0;
        size /= scale;
        /* magic scale number 2.0 comes from my mind, test with my eyes... */
        size *= 2.0;
        updateSubStyle(font, size);
    }
#endif
}

void PlayerEngine::waitLastEnd()
{
    if (MpvProxy *mpv = dynamic_cast<MpvProxy *>(_current)) {
        mpv->pollingEndOfPlayback();
    }else if (QtPlayerProxy *qtPlayer = dynamic_cast<QtPlayerProxy *>(_current)) {
        qtPlayer->pollingEndOfPlayback();
    }
}

void PlayerEngine::onBackendStateChanged()
{
    if (!_current) return;

    CoreState old = _state;
    switch (_current->state()) {
    case Backend::PlayState::Playing:
        _state = CoreState::Playing;
        if (_playlist->count() > 0) {
            m_bAudio = currFileIsAudio();
        }
        //playing . emit thumbnail progress mode signal with setting file
        if (old == CoreState::Idle)
            emit siginitthumbnailseting();
        break;
    case Backend::PlayState::Paused:
        _state = CoreState::Paused;
        break;
    case Backend::PlayState::Stopped:
        _state = CoreState::Idle;
        break;
    }

    updateSubStyles();
    if (old != _state)
        emit stateChanged();

    auto systemEnv = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = systemEnv.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = systemEnv.value(QStringLiteral("WAYLAND_DISPLAY"));
    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        if (_state == CoreState::Idle) {
            QPalette pal(qApp->palette());
            this->setAutoFillBackground(true);
            this->setPalette(pal);
        } else {
            QPalette pal(this->palette());
            pal.setColor(QPalette::Background, Qt::black);
            this->setAutoFillBackground(true);
            this->setPalette(pal);
        }
    }

}

PlayerEngine::CoreState PlayerEngine::state()
{
    auto old = _state;
    switch (_current->state()) {
    case Backend::PlayState::Playing:
        _state = CoreState::Playing;
        break;
    case Backend::PlayState::Paused:
        _state = CoreState::Paused;
        break;
    case Backend::PlayState::Stopped:
        _state = CoreState::Idle;
        break;
    }

    if (old != _state) {
        qWarning() << "###### state mismatch" << old << _state;
        emit stateChanged();
    }
    return _state;
}

const PlayingMovieInfo &PlayerEngine::playingMovieInfo()
{
    static PlayingMovieInfo empty;

    if (!_current) return empty;
    return _current->playingMovieInfo();
}

int PlayerEngine::aid()
{
    if (state() == CoreState::Idle) {
        return 0;
    }
    if (!_current) return 0;

    return _current->aid();
}

int PlayerEngine::sid()
{
    if (state() == CoreState::Idle) {
        return 0;
    }
    if (!_current) return 0;

    return _current->sid();
}

void PlayerEngine::onSubtitlesDownloaded(const QUrl &url, const QList<QString> &filenames,
                                         OnlineSubtitle::FailReason reason)
{
    //mod for warning by xxj ,no any means
    reason = OnlineSubtitle::FailReason::NoError;

    if (state() == CoreState::Idle) {
        return;
    }
    if (!_current) return;

    if (playlist().currentInfo().url != url)
        return;

    bool res = false;

    for (auto &filename : filenames) {
        if (true == _current->loadSubtitle(filename)) {
            res = true;
        } else {
            QFile::remove(filename);
        }
    }

    emit loadOnlineSubtitlesFinished(url, res);

}

bool PlayerEngine::loadSubtitle(const QFileInfo &fi)
{
    if (state() == CoreState::Idle) {
        return true;
    }
    if (!_current) return true;

    const auto &pmf = _current->playingMovieInfo();
    auto pif = playlist().currentInfo();
    int i = 0;
    for (const auto &sub : pmf.subs) {
        if (sub["external"].toBool()) {
            auto path = sub["external-filename"].toString();
            if (path == fi.canonicalFilePath()) {
                this->selectSubtitle(i);
                return true;
            }
        }
        ++i;
    }

    if (_current->loadSubtitle(fi)) {
#ifndef _LIBDMR_
        MovieConfiguration::get().append2ListUrl(pif.url, ConfigKnownKey::ExternalSubs,
                                                 fi.canonicalFilePath());
#endif
        return true;
    }
    return false;
}

void PlayerEngine::loadOnlineSubtitle(const QUrl &url)
{
    if (state() == CoreState::Idle) {
        return;
    }
    if (!_current) return;

    OnlineSubtitle::get().requestSubtitle(url);
}

void PlayerEngine::setPlaySpeed(double times)
{
    if (!_current) return;
    _current->setPlaySpeed(times);
}

void PlayerEngine::setSubDelay(double secs)
{
    if (!_current) return;

    _current->setSubDelay(secs + _current->subDelay());
}

double PlayerEngine::subDelay() const
{
    if (!_current) return 0.0;
    return _current->subDelay();
}

QString PlayerEngine::subCodepage()
{
    if (_current->subCodepage().isEmpty()) {
        return "auto";
    } else {
        return _current->subCodepage();
    }

}

void PlayerEngine::setSubCodepage(const QString &cp)
{
    if (!_current) return;
    _current->setSubCodepage(cp);

//    emit subCodepageChanged();
}

void PlayerEngine::addSubSearchPath(const QString &path)
{
    if (!_current) return;
    _current->addSubSearchPath(path);
}

void PlayerEngine::updateSubStyle(const QString &font, int sz)
{
    if (!_current) return;
    _current->updateSubStyle(font, sz / 2);
}

void PlayerEngine::selectSubtitle(int id)
{
    if (!_current) return;
    if (state() != CoreState::Idle) {
        const auto &pmf = _current->playingMovieInfo();
        if (id >= pmf.subs.size()) return;
        auto sid = pmf.subs[id]["id"].toInt();
        _current->selectSubtitle(sid);
    }
}

bool PlayerEngine::isSubVisible()
{
    if (state() == CoreState::Idle) {
        return false;
    }
    if (!_current) return false;

    return _current->isSubVisible();
}

void PlayerEngine::toggleSubtitle()
{
    if (!_current) return;
    _current->toggleSubtitle();

}

void PlayerEngine::selectTrack(int id)
{
    if (!_current) return;
    _current->selectTrack(id);
}

void PlayerEngine::volumeUp()
{
    if (!_current) return;
    _current->volumeUp();
}

void PlayerEngine::changeVolume(int val)
{
    if (!_current) return;
    _current->changeVolume(val);
}

void PlayerEngine::volumeDown()
{
    if (!_current) return;
    _current->volumeDown();
}

int PlayerEngine::volume() const
{
    if (!_current) return 100;
    return _current->volume();
}

bool PlayerEngine::muted() const
{
    if (!_current) return false;
    return _current->muted();
}

void PlayerEngine::changehwaccelMode(Backend::hwaccelMode hwaccelMode)
{
    if (!_current) return;
    return _current->changehwaccelMode(hwaccelMode);
}

Backend *PlayerEngine::getMpvProxy()
{
    return _current;
}

void PlayerEngine::toggleMute()
{
    if (!_current) return;
    //发送信号通知初始化库函数
    if (!m_bMpvFunsLoad) {
        emit mpvFunsLoadOver();
        m_bMpvFunsLoad = true;
    }

    _current->toggleMute();
    emit volumeChanged();
}

void PlayerEngine::setMute(bool bMute)
{
    _current->setMute(bMute);
}

void PlayerEngine::savePreviousMovieState()
{
    savePlaybackPosition();
}

void PlayerEngine::paintEvent(QPaintEvent *e)
{
    QRect rect = this->rect();
    QPainter p(this);

    if (!CompositingManager::get().composited() || utils::check_wayland_env()) {  // wayland下不会进入mainwindow的paintevent函数导致图标未绘制
        if (_state != Idle && m_bAudio) {
            p.fillRect(rect, QBrush(QColor(0, 0, 0)));
        } else {
            QImage icon = QIcon::fromTheme("deepin-movie").pixmap(130, 130).toImage();;
            QPixmap pix = QPixmap::fromImage(icon);
            QPointF pos = rect.center() - QPoint(pix.width() / 2, pix.height() / 2) / devicePixelRatioF();

            if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
                p.fillRect(rect, QBrush(QColor(255, 255, 255)));
                p.drawPixmap(pos, pix);
            } else {
                p.fillRect(rect, QBrush(QColor(0, 0, 0)));
                p.drawPixmap(pos, pix);
            }
        }
    }
    return QWidget::paintEvent(e);
}

//FIXME: TODO: update _current according to file
void PlayerEngine::requestPlay(int id)
{
    if (!_current) return;
    if (id >= _playlist->count()) return;

    const auto &item = _playlist->items()[id];
    _current->setPlayFile(item.url);

    DRecentData data;
    data.appName = "Deepin Movie";
    data.appExec = "deepin-movie";
    DRecentManager::addItem(item.url.toLocalFile(), data);

    if (_current->isPlayable()) {
        _current->play();
    } else {
        // TODO: delete and try next backend?
    }

    QJsonObject obj{
        {"tid", EventLogUtils::StartPlaying},
        {"version", VERSION},
        {"successful", item.url.isLocalFile() ? "true" : ""},
        {"type", currFileIsAudio() ? "audio" : "video"},
        {"origin", item.url.isLocalFile() ? "local" : "http"},
        {"encapsulation_format", item.mi.fileType},
        {"coding_format",  utils::videoIndex2str(item.mi.vCodecID)}
    };

    EventLogUtils::get().writeLogs(obj);
}

void PlayerEngine::savePlaybackPosition()
{
    if (!_current) return;
    _current->savePlaybackPosition();
}

void PlayerEngine::nextFrame()
{
    if (!_current) return;
    _current->nextFrame();
}

void PlayerEngine::previousFrame()
{
    if (!_current) return;
    _current->previousFrame();
}
void PlayerEngine::makeCurrent()
{
    _current->makeCurrent();
}

void PlayerEngine::play()
{
    if (!_current || !_playlist->count()) return;

    if (state() == CoreState::Paused &&
            getBackendProperty("keep-open").toBool() &&
            getBackendProperty("eof-reached").toBool()) {
        stop();
        next();
    } else if (state() == CoreState::Idle) {
        next();
    }
}

void PlayerEngine::prev()
{
    if (_playingRequest) return;
    _playingRequest = true;
    savePreviousMovieState();
    _playlist->playPrev(true);
    _playingRequest = false;
}

void PlayerEngine::next()
{
    if (_playingRequest) return;
    _playingRequest = true;
    savePreviousMovieState();
    _playlist->playNext(true);
    _playingRequest = false;
}

void PlayerEngine::onPlaylistAsyncAppendFinished(const QList<PlayItemInfo> &pil)
{
    if (_pendingPlayReq.isValid()) {
        auto id = _playlist->indexOf(_pendingPlayReq);
        if (pil.size() && _pendingPlayReq.scheme() == "playlist") {
            id = _playlist->indexOf(pil[0].url);
        }

        if (id >= 0) {
            _playlist->changeCurrent(id);
            _pendingPlayReq = QUrl();
        } else {
            qInfo() << __func__ << "id is:" << id;
        }
        // else, wait for another signal
    } else {
        qInfo() << __func__ << _pendingPlayReq;
    }
}

void PlayerEngine::playByName(const QUrl &url)
{
    savePreviousMovieState();
    int id = _playlist->indexOf(url);
    qInfo() << __func__ << url << "id:" << id;
    if (id >= 0) {
        _playlist->changeCurrent(id);
    } else {
        _pendingPlayReq = url;
    }
}

void PlayerEngine::playSelected(int id)
{
    qInfo() << __func__ << id;
    savePreviousMovieState();
    _playlist->changeCurrent(id);
}

void PlayerEngine::clearPlaylist()
{
    _playlist->clear();
}

void PlayerEngine::pauseResume()
{
    if (!_current) return;
    if (_state == CoreState::Idle)
        return;

    _current->pauseResume();
}

void PlayerEngine::stop()
{
    if (!_current) return;
    _current->stop();
}

bool PlayerEngine::paused()
{
    return _state == CoreState::Paused;
}

QImage PlayerEngine::takeScreenshot()
{
    return _current->takeScreenshot();
}

void PlayerEngine::burstScreenshot()
{
    _current->burstScreenshot();
}

void PlayerEngine::stopBurstScreenshot()
{
    _current->stopBurstScreenshot();
}

void PlayerEngine::seekForward(int secs)
{
    if (state() == CoreState::Idle) return;
    _current->seekForward(secs);
}

void PlayerEngine::seekBackward(int secs)
{
    if (state() == CoreState::Idle) return;
    _current->seekBackward(secs);
}


void PlayerEngine::seekAbsolute(int pos)
{
    if (state() == CoreState::Idle) return;

    _current->seekAbsolute(pos);
}

void PlayerEngine::setDVDDevice(const QString &path)
{
    if (!_current) {
        return;
    }
    _current->setDVDDevice(path);
}

bool PlayerEngine::addPlayFile(const QUrl &url)
{
    QUrl realUrl;

    realUrl = FileFilter::instance()->fileTransfer(url.toString());
    if (!isPlayableFile(realUrl))
        return false;

    _playlist->append(realUrl);
    return true;
}

QList<QUrl> PlayerEngine::addPlayDir(const QDir &dir)
{
    QList<QUrl> valids = FileFilter::instance()->filterDir(dir);

    struct {
        bool operator()(const QUrl& fi1, const QUrl& fi2) const {
            static QRegExp rd("\\d+");
            int pos = 0;
            QString fileName1 = QFileInfo(fi1.toLocalFile()).fileName();
            QString fileName2 = QFileInfo(fi2.toLocalFile()).fileName();
            while ((pos = rd.indexIn(fileName1, pos)) != -1) {
                auto inc = rd.matchedLength();
                auto id1 = fileName1.midRef(pos, inc);

                auto pos2 = rd.indexIn(fileName2, pos);
                if (pos == pos2) {
                    auto id2 = fileName2.midRef(pos, rd.matchedLength());
                    //qInfo() << "id compare " << id1 << id2;
                    if (id1 != id2) {
                        bool ok1, ok2;
                        bool v = id1.toInt(&ok1) < id2.toInt(&ok2);
                        if (ok1 && ok2) return v;
                        return id1.localeAwareCompare(id2) < 0;
                    }
                }

                pos += inc;
            }
            return fileName1.localeAwareCompare(fileName2) < 0;
        }
    } SortByDigits;

    std::sort(valids.begin(), valids.end(), SortByDigits);
    valids = addPlayFiles(valids);
    _playlist->appendAsync(valids);

    return valids;
}

QList<QUrl> PlayerEngine::addPlayFiles(const QList<QUrl> &urls)
{
    qInfo() << __func__;
    QList<QUrl> valids;

    for (QUrl url : urls) {
        if (m_stopRunningThread)
            break;
        if (isPlayableFile(url))
            valids << url;
    }

    _playlist->appendAsync(valids);

    return valids;
}

QList<QUrl> PlayerEngine::addPlayFiles(const QList<QString> &lstFile)
{
    qInfo() << __func__;
    QList<QUrl> valids;
    QUrl realUrl;

    for (QString strFile : lstFile) {
          realUrl = FileFilter::instance()->fileTransfer(strFile);
          if (QFileInfo(realUrl.path()).isDir()) {
              if (realUrl.isLocalFile())          // 保证不是网络路径
                  valids << FileFilter::instance()->filterDir(QDir(realUrl.path()));
          } else {
              valids << realUrl;
          }
    }

    return addPlayFiles(valids);
}

void PlayerEngine::addPlayFs(const QList<QString> &lstFile)
{
    qInfo() << __func__;
    QList<QUrl> valids;
    QUrl realUrl;

    for (QString strFile : lstFile) {
          realUrl = FileFilter::instance()->fileTransfer(strFile);
          if (QFileInfo(realUrl.path()).isDir()) {
              if (realUrl.isLocalFile())          // 保证不是网络路径
                  valids << FileFilter::instance()->filterDir(QDir(realUrl.path()));
          } else {
              valids << realUrl;
          }
    }

    if (valids.isEmpty()) {
        blockSignals(false);
        return;
    }
    QList<QUrl> addFiles = addPlayFiles(valids);
    blockSignals(false);
    emit finishedAddFiles(addFiles);
}

qint64 PlayerEngine::duration() const
{
    if (!_current) return 0;
    return _current->duration();
}

QSize PlayerEngine::videoSize() const
{
    if (!_current) return {0, 0};
    return _current->videoSize();
}

qint64 PlayerEngine::elapsed() const
{
    if (!_current) return 0;
    if (!_playlist) return 0;
    if (_playlist->count() == 0) return 0;
    if (_playlist->current() < 0) return 0;
    qint64 nDuration = _current->duration();        //因为文件信息的持续时间和MPV返回的持续有些差别，所以，我们使用文件返回的持续时间
    qint64 nElapsed = _current->elapsed();
    if (nElapsed < 0)
        return 0;
    if (nElapsed > nDuration)
        return nDuration;
    return nElapsed;
}

void PlayerEngine::setVideoAspect(double r)
{
    if (_current)
        _current->setVideoAspect(r);
}

double PlayerEngine::videoAspect() const
{
    if (!_current) return 0.0;
    return _current->videoAspect();
}

int PlayerEngine::videoRotation() const
{
    if (!_current) return 0;
    return _current->videoRotation();
}

void PlayerEngine::setVideoRotation(int degree)
{
    if (_current)
        _current->setVideoRotation(degree);
}

void PlayerEngine::changeSoundMode(Backend::SoundMode sm)
{
    if (_current)
        _current->changeSoundMode(sm);
}

void PlayerEngine::resizeEvent(QResizeEvent *)
{
#if !defined(USE_DXCB) && !defined(_LIBDMR_)
    bool rounded = !window()->isFullScreen() && !window()->isMaximized();
    if (rounded) {
        QPixmap shape(size());
        shape.fill(Qt::transparent);

        QPainter p(&shape);
        QPainterPath pp;
        pp.addRect(rect());
        p.fillPath(pp, QBrush(Qt::white));
        p.end();

        setMask(shape.mask());
    } else {
        clearMask();
    }
#endif

}

void PlayerEngine::setBackendProperty(const QString &name, const QVariant &val)
{
    if (_current) {
        _current->setProperty(name, val);
    }
}

QVariant PlayerEngine::getBackendProperty(const QString &name)
{
    if (_current) {
        return _current->getProperty(name);
    }
    return QVariant();
}

void PlayerEngine::toggleRoundedClip(bool roundClip)
{
    MpvProxy* pMpvProxy = nullptr;

    pMpvProxy = dynamic_cast<MpvProxy *>(_current);
    if(!pMpvProxy) {
        dynamic_cast<QtPlayerProxy *>(_current)->updateRoundClip(roundClip);
    } else {
        pMpvProxy->updateRoundClip(roundClip);
    }
}

bool PlayerEngine::currFileIsAudio()
{
    bool bAudio = false;
    PlayItemInfo pif;

    if (_playlist->count() > 0) {
        pif = _playlist->currentInfo();
    }

    if (CompositingManager::isMpvExists()) {
        bAudio = pif.thumbnail.isNull() && pif.url.isLocalFile();
    } else {
        bAudio = isAudioFile(pif.url.toString());
    }

    return bAudio;
}

} // end of namespace dmr
