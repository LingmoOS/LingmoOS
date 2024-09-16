// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_PLAYER_ENINE_H
#define _DMR_PLAYER_ENINE_H


#include <QtWidgets>
#include <playlist_model.h>
#include <player_backend.h>
#include <online_sub.h>
#include <QNetworkConfigurationManager>

namespace dmr {
class PlaylistModel;

using SubtitleInfo = QMap<QString, QVariant>;
using AudioInfo = QMap<QString, QVariant>;

struct PlayingMovieInfo {
    QList<SubtitleInfo> subs;
    QList<AudioInfo> audios;
};

class PlayerEngine: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qint64 duration READ duration)
    Q_PROPERTY(qint64 elapsed READ elapsed NOTIFY elapsedChanged)
    Q_PROPERTY(QSize videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(bool paused READ paused)

    Q_PROPERTY(CoreState state READ state NOTIFY stateChanged)
public:
    enum CoreState {
        Idle,
        Playing,
        Paused,
    };
    Q_ENUM(CoreState)

    // filetypes supported by mpv: https://github.com/mpv-player/mpv/blob/master/player/external_files.c
    const static QStringList audio_filetypes;
    const static QStringList video_filetypes;

    const static QStringList subtitle_suffixs;

    /* backend like mpv will asynchronously report end of playback.
     * there are situations when we need to see the end-event before
     * proceed (e.g playlist next)
     */
    void waitLastEnd();

    friend class PlaylistModel;

    explicit PlayerEngine(QWidget *parent);
    virtual ~PlayerEngine();

    // only the last dvd device set
    void setDVDDevice(const QString &path);
    //add by heyi
    //第一次播放需要初库始化函数指针
    void firstInit();

    bool addPlayFile(const QUrl &url);
    // return collected valid urls
    QList<QUrl> addPlayDir(const QDir &dir);
    // returned list contains only accepted valid items
    QList<QUrl> addPlayFiles(const QList<QUrl> &urls);
    /**
     * @brief addPlayFiles 添加播放文件
     * @param 文件集合
     * @return 返回已添加成功的文件
     */
    QList<QUrl> addPlayFiles(const QList<QString> &lstFile);
    /**
     * @brief addPlayFs 在线程中运行添加文件
     * @param 文件集合
     */
    void addPlayFs(const QList<QString> &lstFile);
    /**
     * @brief isPlayableFile 判断一个文件是否可以播放
     * @param url 文件url
     * @return 是否可以播放
     */
    bool isPlayableFile(const QUrl &url);
    /**
     * @brief isPlayableFile 判断一个文件是否可以播放
     * @param url 文件路径
     * @return 是否可以播放
     */
    bool isPlayableFile(const QString &name);
    static bool isAudioFile(const QString &name);
    static bool isSubtitle(const QString &name);

    // only supports (+/-) 0, 90, 180, 270
    int videoRotation() const;
    void setVideoRotation(int degree);

    void setVideoAspect(double r);
    double videoAspect() const;

    qint64 duration() const;
    qint64 elapsed() const;
    QSize videoSize() const;
    const struct MovieInfo &movieInfo();

    bool paused();
    CoreState state();
    const PlayingMovieInfo &playingMovieInfo();
    void setPlaySpeed(double times);

    void loadOnlineSubtitle(const QUrl &url);
    bool loadSubtitle(const QFileInfo &fi);
    void toggleSubtitle();
    bool isSubVisible();
    void selectSubtitle(int id); // id into PlayingMovieInfo.subs
    int sid();
    void setSubDelay(double secs);
    double subDelay() const;
    void updateSubStyle(const QString &font, int sz);
    void setSubCodepage(const QString &cp);
    QString subCodepage();
    void addSubSearchPath(const QString &path);

    void selectTrack(int id); // id into PlayingMovieInfo.audios
    int aid();

    void changeSoundMode(Backend::SoundMode sm);
    int volume() const;
    bool muted() const;

    void changehwaccelMode(Backend::hwaccelMode hwaccelMode);

    PlaylistModel &playlist() const
    {
        return *_playlist;
    }

    Backend * getMpvProxy();

    PlaylistModel *getplaylist()
    {
        return _playlist;
    };

    QImage takeScreenshot();
    void burstScreenshot(); //initial the start of burst screenshotting
    void stopBurstScreenshot();

    void savePlaybackPosition();

    void nextFrame();
    void previousFrame();
    //只在wayland下opengl窗口使用
    void makeCurrent();

    // use with caution
    void setBackendProperty(const QString &, const QVariant &);
    QVariant getBackendProperty(const QString &);

    void toggleRoundedClip(bool roundClip);
    bool currFileIsAudio();

signals:
    void tracksChanged();
    void elapsedChanged();
    void videoSizeChanged();
    void stateChanged();
    void fileLoaded();
    void muteChanged();
    void volumeChanged();
    void sidChanged();
    void aidChanged();
    void subCodepageChanged();

    void loadOnlineSubtitlesFinished(const QUrl &url, bool success);
    //add by heyi mpv函数加载完毕
    void mpvFunsLoadOver();

    //emit during burst screenshotting
    void notifyScreenshot(const QImage &frame, qint64 time);

    void playlistChanged();

    void onlineStateChanged(const bool isOnline);
    void mpvErrorLogsChanged(const QString prefix, const QString text);
    void mpvWarningLogsChanged(const QString prefix, const QString text);
    void urlpause(bool status);

    void siginitthumbnailseting();
    void updateDuration();
    void sigInvalidFile(QString strFileName);

    void sigMediaError();
    void finishedAddFiles(QList<QUrl>);

public slots:
    void play();
    void pauseResume();
    void stop();

    void prev();
    void next();
    void playSelected(int id); // id as in playlist indexes
    void playByName(const QUrl &url);
    void clearPlaylist();

    void seekForward(int secs);
    void seekBackward(int secs);
    void seekAbsolute(int pos);

    void volumeUp();
    void volumeDown();
    void changeVolume(int val);
    void toggleMute();
    void setMute(bool bMute);

protected slots:
    void onBackendStateChanged();
    void requestPlay(int id);
    void updateSubStyles();
    void onSubtitlesDownloaded(const QUrl &url, const QList<QString> &filenames,
                               OnlineSubtitle::FailReason);
    void onPlaylistAsyncAppendFinished(const QList<PlayItemInfo> &);

protected:
    PlaylistModel *_playlist {nullptr};
    CoreState _state { CoreState::Idle };
    Backend *_current {nullptr};

    QUrl _pendingPlayReq;

    bool _playingRequest {false};
    //add by heyi
    bool m_bMpvFunsLoad {false};

    void resizeEvent(QResizeEvent *) override;
    void savePreviousMovieState();

    void paintEvent(QPaintEvent *e) override;

private:
    QNetworkConfigurationManager _networkConfigMng;
    bool m_bAudio;
    bool m_stopRunningThread;
};
}

#endif /* ifndef _DMR_PLAYER_ENINE_H */
