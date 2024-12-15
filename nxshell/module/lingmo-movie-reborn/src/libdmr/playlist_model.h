// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_PLAYLIST_MODEL_H
#define _DMR_PLAYLIST_MODEL_H

#include <QtWidgets>
//#include <QtConcurrent>
#include <DApplicationHelper>

#include "utils.h"
#include <QNetworkReply>
#include <QMutex>

#define THUMBNAIL_SIZE 500
#define SEEK_TIME "00:00:01"

namespace dmr {
class PlayerEngine;
class LoadThread;
class GetThumanbil;

struct MovieInfo {
    bool valid;
    QString title;
    QString fileType;
    QString resolution;
    QString filePath;
    QString creation;

    // rotation in metadata, this affects width/height
    int raw_rotate;
    qint64 fileSize;
    qint64 duration;
    int width = -1;
    int height = -1;
    //3.4添加视频信息
    //视频流信息
    int vCodecID;
    qint64 vCodeRate;
    int fps;
    float proportion;
    //音频流信息
    int aCodeID;
    qint64 aCodeRate;
    int aDigit;
    int channels;
    int sampling;
#ifdef _MOVIE_USE_
    QString strFmtName; // 文件封装名
#endif
    MovieInfo() {
        valid = false;
        raw_rotate = -1;
        fileSize = -1;
        duration = -1;
        width = -1;
        height = -1;
        vCodecID = -1;
        aCodeRate = -1;
        fps = -1;
        proportion = -1.0;
        aCodeID = -1;
        aCodeRate = -1;
        aDigit = -1;
        channels = -1;
        sampling = -1;
    }

    static struct MovieInfo parseFromFile(const QFileInfo &fi, bool *ok = nullptr);
    QString durationStr() const
    {
        return utils::Time2str(duration);
    }

    QString videoCodec() const
    {
        return  utils::videoIndex2str(vCodecID);
    }

    QString audioCodec() const
    {
        return utils::audioIndex2str(aCodeID);
    }

    //获取字幕编码格式（备用）
    /*QString subtitleCodec() const
    {
        return utils::subtitleIndex2str();
    }*/

    QString sizeStr() const
    {
        auto K = 1024;
        auto M = 1024 * K;
        auto G = 1024 * M;
        if (fileSize > G) {
            return QString(QT_TR_NOOP("%1G")).arg((double)fileSize / G, 0, 'f', 1);
        } else if (fileSize > M) {
            return QString(QT_TR_NOOP("%1M")).arg((double)fileSize / M, 0, 'f', 1);
        } else if (fileSize > K) {
            return QString(QT_TR_NOOP("%1K")).arg((double)fileSize / K, 0, 'f', 1);
        }
        return QString(QT_TR_NOOP("%1")).arg(fileSize);
    }
    /**
     * @brief 判断是否是H.264裸流，因为没有时长等信息所以需要对此类型单独判断
     * @return 是否是裸流
     */
    bool isRawFormat() const
    {
        bool bFlag = false;
#ifdef _MOVIE_USE_
        if(strFmtName.contains("raw",Qt::CaseInsensitive))
            bFlag = true;
#endif

        return bFlag;
    }
};


struct PlayItemInfo {
    bool valid;
    bool loaded;  // if url is network, this is false until playback started
    QUrl url;
    QFileInfo info;
    QPixmap thumbnail;
    QPixmap thumbnail_dark;
    struct MovieInfo mi;

    bool refresh();
};

using AppendJob = QPair<QUrl, QFileInfo>; // async job
using PlayItemInfoList = QList<PlayItemInfo>;
using UrlList = QList<QUrl>;


class PlaylistModel: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int current READ current WRITE changeCurrent NOTIFY currentChanged)

public:
    friend class PlayerEngine;
    enum PlayMode {
        OrderPlay,
        ShufflePlay,
        SinglePlay,
        SingleLoop,
        ListLoop,
    };

    void stop();

    PlayMode playMode() const;
    void setPlayMode(PlayMode pm);

    explicit PlaylistModel(PlayerEngine *engine);
    ~PlaylistModel();

    qint64 getUrlFileTotalSize(QUrl url, int tryTimes) const;

    /**
     * @brief clear 清空播放列表
     */
    void clear();
    /**
     * @brief remove 移除播放列表中的选定项
     * @param pos 传入的删除项
     */
    void remove(int pos);
    void append(const QUrl &);

    void appendAsync(const QList<QUrl> &);
    void collectionJob(const QList<QUrl> &, QList<QUrl> &);

    void playNext(bool fromUser);
    void playPrev(bool fromUser);

    int count() const;
    const QList<PlayItemInfo> &items() const
    {
        return _infos;
    }
    QList<PlayItemInfo> &items()
    {
        return _infos;
    }

    int current() const;
    const PlayItemInfo &currentInfo() const;
    PlayItemInfo &currentInfo();
    int size() const;
    int indexOf(const QUrl &url);

    void switchPosition(int p1, int p2);

//    bool hasPendingAppends();
    void handleAsyncAppendResults(QList<PlayItemInfo> &pil);
    struct PlayItemInfo calculatePlayInfo(const QUrl &, const QFileInfo &fi, bool isDvd = false);
    bool getthreadstate();
    void savePlaylist();
    void clearPlaylist();
    QList<QUrl> getLoadList()
    {
        return m_loadFile;
    };
    void loadPlaylist();
    /**
     * @brief getThumanbilRunning 获取加载线程是否运行
     * @return 返回是否正在运行
     */
    bool getThumanbilRunning();

    //获取视频信息
    MovieInfo getMovieInfo(const QUrl &url, bool *is);

    //获取视频首帧图片
    QImage getMovieCover(const QUrl &url);

public slots:
    void changeCurrent(int);
    void delayedAppendAsync(const QList<QUrl> &);
//    void deleteThread();
    void clearLoad();

private slots:
//    void onAsyncAppendFinished();
    void onAsyncFinished();
    void onAsyncUpdate(PlayItemInfo);
    void slotStateChanged();


signals:
    void countChanged();
    void currentChanged();
    void itemRemoved(int);
    void itemsAppended();
    void emptied();
    void playModeChanged(PlayMode);
    void asyncAppendFinished(const QList<PlayItemInfo> &);
    void itemInfoUpdated(int id);
    void updateDuration();

private:
    void initThumb();
    void initFFmpeg();
    bool getMusicPix(const QFileInfo &fi, QPixmap &rImg);
    struct MovieInfo parseFromFile(const QFileInfo &fi, bool *ok = nullptr);
    struct MovieInfo parseFromFileByQt(const QFileInfo &fi, bool *ok = nullptr);
    QString libPath(const QString &strlib);
    // when app starts, and the first time to load playlist
    bool _firstLoad {true};
    int _count {0};
    int _current {-1};
    int _last {-1};
    bool _hasNormalVideo{false};
    PlayMode _playMode {PlayMode::OrderPlay};
    QList<PlayItemInfo> _infos;

    QList<int> _playOrder; // for shuffle mode
    int _shufflePlayed {0}; // count currently played items in shuffle mode
    int _loopCount {0}; // loop count

    QList<AppendJob> _pendingJob; // async job
    QSet<QString> _urlsInJob;  // url list

    QQueue<UrlList> _pendingAppendReq;

    bool _userRequestingItem {false};

    PlayerEngine *_engine {nullptr};

    QString _playlistFile;

    LoadThread *m_ploadThread;
    GetThumanbil *m_getThumanbil {nullptr};
    QMutex *m_pdataMutex;
    bool m_brunning;
    QList<QUrl> m_tempList;
    QList<QUrl> m_loadFile;
    bool m_initFFmpeg {false};
    bool m_bInitThumb {false};

    void reshuffle();
    void appendSingle(const QUrl &);
    void tryPlayCurrent(bool next);
};


class LoadThread: public QThread
{
    Q_OBJECT

public:
    LoadThread(PlaylistModel *model, const QList<QUrl> &urls);
    ~LoadThread();

public:
    void run();

private:
    PlaylistModel *_pModel;
    QList<QUrl> _urls;

    QList<AppendJob> _pendingJob; // async job
    QSet<QString> _urlsInJob;  // url list
};

class GetThumanbil : public QThread
{
    Q_OBJECT
public:
    GetThumanbil(PlaylistModel *model, const QList<QUrl> &urls): m_model(model), m_urls(urls)
    {
//        m_model = model;
//        m_urls = urls;
        m_mutex = new QMutex;
    };
    ~GetThumanbil()
    {
        m_stop = true;
        delete m_mutex;
        m_mutex = nullptr;
    };
    //QList<PlayItemInfo> getInfoList() {return m_itemInfo;}
    void stop()
    {
        m_stop = true;
    };
    void setUrls(QList<QUrl> urls)
    {
        m_mutex->lock();
        m_urls = urls;
        m_mutex->unlock();
    };
    void clearItem()
    {
        m_mutex->lock();
        //m_itemInfo.clear();
        m_urls.clear();
        m_mutex->unlock();
    };

    void run()
    {
        m_mutex->lock();
        QList<QUrl> urls = m_urls;
        m_mutex->unlock();
        foreach (QUrl url, urls) {
            QFileInfo info(url.path());
            //m_itemInfo.append();
            emit updateItem(m_model->calculatePlayInfo(url, info, false));
            if (m_stop)
                break;
        }
    }

signals:
    void updateItem(PlayItemInfo);
private:
    PlaylistModel *m_model;
    QList<QUrl> m_urls;
    //QList<PlayItemInfo> m_itemInfo;
    QMutex *m_mutex;
    bool m_stop {false};
};

}

#endif /* ifndef _DMR_PLAYLIST_MODEL_H */

