#include "mmediaplayer.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <lingmo-log4qt.h>

MMediaPlayer::MMediaPlayer(QObject *parent)
    : QObject(parent)
{
    createMvpplayer();
}

void MMediaPlayer::setPlaylist(MMediaPlaylist *playlist)
{
    //异常情况：空指针
    if (playlist == nullptr) {
        return;
    }
    m_playList = playlist;
    connect(this,&MMediaPlayer::playFinish,m_playList,&MMediaPlaylist::palyFinish,Qt::UniqueConnection);
    connect(this,&MMediaPlayer::playError,m_playList,&MMediaPlaylist::playError,Qt::UniqueConnection);
    connect(m_playList,&MMediaPlaylist::autoPlay,this,&MMediaPlayer::autoPlay,Qt::UniqueConnection);
    connect(m_playList,&MMediaPlaylist::stop,this,&MMediaPlayer::stop,Qt::UniqueConnection);
    connect(this,&MMediaPlayer::playErrorMsg,m_playList,&MMediaPlaylist::playErrorMsg,Qt::UniqueConnection);
}

void MMediaPlayer::truePlay(QString startTime)
{
    //异常情况：入参不合法
    if (startTime.isEmpty()) {
        return;
    }
    //异常情况：播放列表空指针
    if (m_playList == nullptr) {
        return;
    }

    qDebug() << "startTime: " << startTime;

    QString filePath = m_playList->getPlayFileName();
    qDebug() << "play filePath : " << filePath << QUrl(filePath).toLocalFile();;

    //异常情况：本地文件不存在
    if (!QFileInfo::exists(QUrl(filePath).toLocalFile())) {
        qDebug() << "当前播放文件不存在！" << filePath << QUrl(filePath).toLocalFile();
        Q_EMIT playErrorMsg(NotFound);
        Q_EMIT playError();
        return;
    }

    qDebug() << "当前播放文件存在： " << filePath;
    const QByteArray c_filename = filePath.toUtf8();
    //如果文件名和上次一样，且不是因为拖动进度条播放，说明上次是暂停
    if (c_filename == filenameBack && m_positionChangeed == false) {
        if (filenameBack != "") {
            //切换播放状态为播放
            pause();
        }
        return;
    }
    //重置参数
    m_position = 0;
    m_positionChangeed = false;
    setProperty("start",startTime);
    setProperty("force-seekable", "yes");
    const char *args[] = {"loadfile",c_filename, NULL};
    mpv_command_async(m_mpvPlayer, 0, args);

    //如果不播放任何媒体，则切换状态为停止
    if (c_filename == "") {
        changeState(StoppedState);
        return;
    }
    //记录到上次播放变量中
    filenameBack = c_filename;
    //切换播放状态为正在播放
    changeState(PlayingState);
}

void MMediaPlayer::play()
{
    //从开头开始播放
    truePlay("0");
}

void MMediaPlayer::setPlayRate(double s)
{
    setProperty("speed", QString::number(s));
}

double MMediaPlayer::getPlayRate()
{
    return getProperty("speed").toDouble();
}

void MMediaPlayer::pause()
{
    // 获得mpv播放器的"暂停"状态
    QString pasued = getProperty("pause");

    KyInfo() << "pauseState = " << pasued;

    // 根据"暂停"状态来选择暂停还是播放
    if(pasued == "no") {
        KyInfo() << "set pause yes";

        setProperty("pause", "yes");
        changeState(PausedState);
    } else if(pasued == "yes") {
        KyInfo() << "set pause no";

        setProperty("pause", "no");
        changeState(PlayingState);
    }
}

void MMediaPlayer::pauseOnly()
{
    // 获得mpv播放器的"暂停"状态
    QString pasued = getProperty("pause");
    KyInfo() << "pauseStated = " << pasued;

    // 根据"暂停"状态来选择暂停还是播放
    if(pasued == "no") {
        KyInfo() << "begin pause.";
        setProperty("pause", "yes");
        changeState(PausedState);
    }
}

void MMediaPlayer::stop()
{
    filenameBack = "";
    setProperty("pause", "no");
    const char *args[] = {"loadfile", "", NULL};
    mpv_command_async(m_mpvPlayer, 0, args);
    changeState(StoppedState);
}

MMediaPlayer::State MMediaPlayer::state() const
{
    return m_state;
}

qint64 MMediaPlayer::position() const
{
    return m_position;
}

void MMediaPlayer::setPosition(qint64 pos)
{
    double sec = double(pos)/1000;
    m_positionChangeed = true;
    //记录拖动进度条之前播放状态是否为暂停
    bool restartPlay = false;
    if (m_state == PausedState) {
        restartPlay = true;
    }
    //从拖动完成的位置开始播放
    truePlay(QString::number(sec));

    if (restartPlay) {
        //切换播放状态为播放
        pause();
    }
}

void MMediaPlayer::setVolume(int vol)
{
//    setProperty("volume",QString::number(vol));
//    Q_EMIT signalVolume(vol);
    // 设置音量，此音量和系统同步，不单独设置mpv音量

//    delayMsecond(100);

    QDBusMessage message = QDBusMessage::createSignal("/", "org.lingmo.music", "sinkInputVolumeChanged");
    message << "lingmo-music" << vol << false;

    KyInfo() << "createSignal: volume = " << vol;
    QDBusConnection::sessionBus().send(message);
}

qint64 MMediaPlayer::duration() const
{
    return m_duration;
}

void MMediaPlayer::setMedia(const MMediaContent &media)
{
    QUrl url =media.canonicalUrl();
    //防止内存泄漏
    if (m_tmpPlayList != nullptr) {
        m_tmpPlayList->deleteLater();
    }
    //创建新的播放列表并将歌曲录入
    m_tmpPlayList = new MMediaPlaylist(this);
    m_tmpPlayList->addMedia(url);
    setPlaylist(m_tmpPlayList);
    //以暂停状态从头开始播放
    setProperty("pause", "yes");
    play();
}

bool MMediaPlayer::isAvailable() const
{
    return true;
}

void MMediaPlayer::delayMsecond(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}


void MMediaPlayer::onMpvEvents()
{
    //处理所有事件，直到事件队列为空
    while (m_mpvPlayer)
    {
        mpv_event *event = mpv_wait_event(m_mpvPlayer, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MMediaPlayer::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
        case MPV_EVENT_PROPERTY_CHANGE: { //属性改变事件
            mpv_event_property *prop = (mpv_event_property *)event->data;
            //播放时，时间改变事件
            if (strcmp(prop->name, "time-pos") == 0) {
                if (prop->format == MPV_FORMAT_DOUBLE) {
                    //将播放状态设置为播放中
                    if (m_state == StoppedState) {
                        changeState(PlayingState);
                    }
                    // 获得播放时间
                    double time = *(double *)prop->data;
                    //将单位换算为毫秒
                    m_position = time * 1000;
                    Q_EMIT positionChanged(m_position);
                } else if (prop->format == MPV_FORMAT_NONE) {
                    //当前时长距离总时长不超过500毫秒判断播放结束
                    if ( m_duration!=0 && (m_duration - m_position < 500)) {
                        m_duration = 0;
                        m_position = 0;
                        //播放结束
                        Q_EMIT playFinish();
                    } else {
                        //切歌
                        changeState(StoppedState);
                    }
                }
            }
        }
        break;
    case MPV_EVENT_PLAYBACK_RESTART:{ //初始化完成事件
        //获取总时长
        m_duration = getProperty("duration").toDouble() *1000;//单位换算为毫秒
        Q_EMIT durationChanged(m_duration);
    }
        break;
    case MPV_EVENT_IDLE:{ //播放器空闲事件，只有刚启动时、播放完成时、歌曲异常时会进入此分支
        QString playlist = getProperty("playlist");
        if (!playlist.contains(',')) { //排除播放完成
            if (playlist.length() > 2) { //排除刚启动
                //歌曲播放异常
                Q_EMIT playErrorMsg(Damage);
            }
        }
    }
        break;
        //MPV会概率错误的发送此信号，导致没播放完也跳转到下一首
//    case MPV_EVENT_END_FILE:{ //播放结束事件
//        if (m_position != 0) {
//            //重置参数
//            m_duration = 0;
//            m_position = 0;
//            //播放结束
//            Q_EMIT playFinish();
//        }
//    }
//        break;
        default: ;
    }
}

// 回调函数
static void wakeup(void *ctx)
{
    // 此回调可从任何mpv线程调用（但也可以从调用mpv API的线程递归地返回）
    // 只是需要通知要唤醒的Qt GUI线程（以便它可以使用mpv_wait_event（）），并尽快返回
    MMediaPlayer *mvpPlayer = (MMediaPlayer *)ctx;
    Q_EMIT mvpPlayer->mpvEvents();
}

void MMediaPlayer::createMvpplayer()
{
    // 创建mpv实例
    setlocale(LC_NUMERIC,"C");
    m_mpvPlayer = mpv_create();
    if (m_mpvPlayer == nullptr) {
        qDebug()<<"创建播放模块失败！";
        this->deleteLater();
        return;
    }
    //禁用视频流
    setProperty("vid", "no");
    //接收事件
    connect(this, &MMediaPlayer::mpvEvents, this, &MMediaPlayer::onMpvEvents, Qt::QueuedConnection);
    mpv_set_wakeup_callback(m_mpvPlayer, wakeup, this);
    //绑定事件
    mpv_observe_property(m_mpvPlayer, 0, "time-pos", MPV_FORMAT_DOUBLE);
    // 判断mpv实例是否成功初始化
    if (mpv_initialize(m_mpvPlayer) < 0) {
        qDebug()<<"初始化失败！";
        this->deleteLater();
    }
}

void MMediaPlayer::setProperty(const QString &name, const QString &value)
{
    mpv_set_option_string(m_mpvPlayer, name.toLatin1().data(), value.toLatin1().data());
}

QString MMediaPlayer::getProperty(const QString &name) const
{
    return (QString)mpv_get_property_string(m_mpvPlayer, name.toLatin1().data());
}

void MMediaPlayer::changeState(MMediaPlayer::State stateNow)
{
    //待设置的循环模式和设置之前一致则不处理
    if (m_state == stateNow ) {
        return;
    }
    m_state = stateNow;
    Q_EMIT stateChanged(m_state);
}

void MMediaPlayer::autoPlay(MMediaPlaylist::PlaybackMode playbackMode)
{
    //如果是单曲循环模式
    if (playbackMode == MMediaPlaylist::PlaybackMode::CurrentItemInLoop) {
        //播放完毕自动切歌（借用播放点改变时间逻辑循环）
        m_positionChangeed = true;
    }
    truePlay("0");
}
