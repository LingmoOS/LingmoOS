#ifndef MMediaPlaylist_H
#define MMediaPlaylist_H

#include <QObject>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QDateTime>

class MMediaContent
{

public:
    MMediaContent(QUrl url);
    QUrl canonicalUrl() const; //获取Qurl内容
private:
    QUrl m_url; //媒体url路径
};


class MMediaPlaylist : public QObject
{
    Q_OBJECT
public:
    //                      只播放一次           单曲循环         列表播放   列表循环 随机播放
    enum PlaybackMode { CurrentItemOnce=0, CurrentItemInLoop, Sequential, Loop,  Random }; //播放循环模式枚举
    MMediaPlaylist(QObject *parent = nullptr);

    QString getPlayFileName(); //获取待播放媒体的文件名
    int currentIndex() const; //获取当前播放的歌曲在播放列表中的位置
    bool addMedia(const QUrl &items); //添加媒体
    void next(); //切换下一首
    void previous(); //切换上一首
    void setCurrentIndex(int index); //设置选中播放列表中的位置
    PlaybackMode playbackMode() const; //获取播放循环模式
    void setPlaybackMode(PlaybackMode mode); //设置循环播放模式
    int mediaCount() const; //列表中总媒体的数量
    MMediaContent media(int index) const; //获取列表中特定位置的媒体
    bool clear(); //清空列表
    bool removeMedia(int pos); //移出特定位置的歌曲
public Q_SLOTS:
    void palyFinish(); //播放完成槽函数
    void playError(); //播放异常槽函数
    void playErrorMsg(int Damage); //播放错误异常
private:
    int randomIndex(); //生成随机数
    QList<QUrl> m_playerList; //实际的播放队列
    int m_index = 0; //当前播放的歌曲位置
    int notExistFileNumber = 0; // 播放列表中不存在的文件数目，用于判断当前歌单列表在“循环”或“随机”模式下是否需要继续播放
    PlaybackMode m_playbackMode = Loop; //当前列表的循环模式
Q_SIGNALS:
    void currentIndexChanged(int); //媒体切换信号
    void playbackModeChanged(MMediaPlaylist::PlaybackMode mode); //播放循环模式切换信号
    void autoPlay(MMediaPlaylist::PlaybackMode playbackMode); //自动播放下一首信号
    void stop(); //停止播放信号
};

#endif // MMediaPlaylist_H
