#include <QDebug>
#include <QDBusConnection>
#include <lingmo-log4qt.h>
#include "player.h"
#include "UIControl/base/musicDataBase.h"
#include "UIControl/base/musicfileinformation.h"
#include "UI/player/playsongarea.h"
#include "UI/base/widgetstyle.h"
#include "UI/dbusadapter.h"

bool playController::play(QString playlist, int index)
{
    if (playlist.compare(m_curList)==0)
    {
//        if (index == m_curIndex) {
//            if (m_player->state() == MMediaPlayer::State::PlayingState){
//                play();
//            }else {
//                pause();
//            }
//        }
//        else {
            stop();
            setSongIndex(index);
            play();
//        }
        return true;
    }
    return false;
}

bool playController::play()
{
    if (m_player == nullptr) {
        return false;
    }
    if (m_player->isAvailable() == false) { //存疑
        return false;
    }
    if (m_player->state() == MMediaPlayer::State::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
        Q_EMIT curIndexChanged(m_playlist->currentIndex());
    }

    return true;
}

bool playController::pause()
{
    if (m_player == nullptr) {
        return false;
    }
    m_player->pause();

    return true;
}

bool playController::pauseOnly()
{
    if (m_player == nullptr) {
        return false;
    }
    m_player->pauseOnly();

    return true;
}

bool playController::stop()
{
    if (m_player == nullptr) {
        return false;
    }
    m_player->stop();

    return true;
}

void playController::setSongIndex(int index)
{
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return;
    }
    if(index < 0)
    {
        return;
    }
    if (index > m_playlist->mediaCount()) {
        return;
    }
    m_playlist->setCurrentIndex(index);
    Q_EMIT curIndexChanged(index);
}
int playController::songIndex()
{
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return -1;
    }

    return m_playlist->currentIndex();
}

int playController::playmode()
{
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return -1;
    }

    return static_cast<MMediaPlaylist::PlaybackMode>(m_playlist->playbackMode());
}
void playController::setPlaymode(int mode)
{
    if (m_playlist == nullptr || m_player == nullptr) {
        return;
    }
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return;
    }

//    if (mode < 0 || mode > 5) {
//        m_playlist->setPlaybackMode(MMediaPlaylist::PlaybackMode::CurrentItemOnce);
//    } else
        m_playlist->setPlaybackMode(static_cast<MMediaPlaylist::PlaybackMode>(mode));
}

void playController::curPlaylist()
{
    // Print Playlist information
    if (m_playlist == nullptr) {
        return;
    }

    for (auto i = 0; i < m_playlist->mediaCount(); i++) {
        MMediaContent content = m_playlist->media(i);
//        qDebug() << "   "
//                 << "media[" << i << "] is:" << content.canonicalUrl();
    }
}
void playController::setCurPlaylist(QString name, QStringList songPaths)
{
    if (m_curList.compare(name)==0)
    {
//        qDebug() << "setCurPlaylist m_curList.compare(name)==0" << m_curList << name;
//        return ;
    }
    if (m_playlist == nullptr || m_player == nullptr) {
        return;
    }
    disconnect(m_playlist,&MMediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    m_curList = name;
    m_playlist->clear();

    for (auto path : songPaths) {
        m_playlist->addMedia(QUrl::fromLocalFile(path));
    }
    m_player->stop();
    m_player->setPlaylist(nullptr);
    m_player->setPlaylist(m_playlist);


//    m_playlist->setCurrentIndex(-1);
    connect(m_playlist,&MMediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    isInitialed = true;
}
void playController::addSongToCurList(QString name, QString songPath)
{
    if (name.compare(m_curList) != 0) {
        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }
    if (m_playlist != nullptr) {
        m_playlist->addMedia(QUrl::fromLocalFile(songPath));
    }
}
void playController::removeSongFromCurList(QString name, int index)
{
    if (name.compare(m_curList) != 0)
    {
//        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }
    if (m_playlist != nullptr) {
//        m_playlist->removeMedia(index);
        //判断删除后 播放歌曲的index    当前只判断了删除正在播放的歌曲    还没做删除正在播放之前的歌曲和之后的歌曲
            int count = m_playlist->mediaCount();

            int state = m_player->state();

            if(m_curIndex == index)
            {
                stop();
                if(m_curIndex == count - 1)
                {
                    m_curIndex = 0;
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                }
                else
                {
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
//                    setSongIndex(m_curIndex);
                    setSongIndex(m_curIndex);
                }
                //删除当前播放歌曲不更改播放状态  2021.09.10
                if (state == MMediaPlayer::State::PlayingState) {
                    m_player->play();
                } else {                            //设置进度条归 0
                    Q_EMIT signalSetValue();
                    m_player->pause();
                }
            }
            else if(m_curIndex > index)
            {
//                int position = 0;
//                if(m_player->state()==MMediaPlayer::PlayingState)
//                {
//                    position = m_player->position();
//                }
//                m_player->stop();
                m_playlist->removeMedia(index);
                m_curIndex = m_curIndex - 1;
                //只负责高亮歌曲
                setSongIndex(m_curIndex);
//                m_player->setPosition(position);
                //歌曲删除重新播放(跨函数调用)
//                PlaySongArea::getInstance().hSlider->setValue(position);
                //MPV setPosition已经设置播放，不用再次设置播放
//                m_player->play();
            }
            else if(m_curIndex < index)
            {
                m_playlist->removeMedia(index);
            }

            Q_EMIT curIndexChanged(m_curIndex);
            Q_EMIT currentIndexAndCurrentList(m_curIndex,m_curList);
            slotIndexChange(m_curIndex);
//        auto cr_index = m_playlist->currentIndex();
//        Q_EMIT curIndexChanged(cr_index);
//        Q_EMIT currentIndexAndCurrentList(cr_index,m_curList);
        //删除正在播放的歌曲时，正在播放的歌曲名和时长实时更新
//        slotIndexChange(cr_index);
//        if(name == ALLMUSIC)
//        {

//        }
    }
}


void playController::removeSongFromLocalList(QString name, int index)
{
    if (name.compare(m_curList) != 0)
    {
        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }
    if (m_playlist != nullptr)
    {
            int count = m_playlist->mediaCount();

            int state = m_player->state();

            if(m_curIndex == index)
            {
                stop();
                if(m_curIndex == count - 1)
                {
                    m_curIndex = 0;
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                }
                else
                {
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                }
                //删除当前播放歌曲不更改播放状态  2021.09.10
                if (state == MMediaPlayer::State::PlayingState) {
                    m_player->play();
                } else {                            //设置进度条归 0
                    Q_EMIT signalSetValue();
                    m_player->pause();
                }
            }
            else if(m_curIndex > index)
            {
//                int position = 0;
//                if(m_player->state()==MMediaPlayer::PlayingState)
//                {
//                    position = m_player->position();
//                }
//                m_player->stop();
                m_playlist->removeMedia(index);
                m_curIndex = m_curIndex - 1;
                setSongIndex(m_curIndex);
//                m_player->setPosition(position);
//                m_player->play();
            }
            else if(m_curIndex < index)
            {
                m_playlist->removeMedia(index);
            }

            Q_EMIT curIndexChanged(m_curIndex);
            Q_EMIT currentIndexAndCurrentList(m_curIndex,m_curList);
            slotIndexChange(m_curIndex);
    }
}

playController::PlayState playController::getState()
{
    if(m_player->state() == MMediaPlayer::State::PlayingState)
        return PlayState::PLAY_STATE;
    else if(m_player->state() == MMediaPlayer::State::PausedState)
        return PlayState::PAUSED_STATE;
    else if(m_player->state() == MMediaPlayer::State::StoppedState)
        return PlayState::STOP_STATE;
}

playController::playController()
    : m_curList(""),m_curIndex(-1)
{
    m_player = new MMediaPlayer(this);
    if (m_player == nullptr) {
        qDebug() << "failed to create player ";
        return;
    }
    m_playlist = new MMediaPlaylist(m_player);
    if (m_playlist == nullptr) {
        qDebug() << "failed to create laylist";
        return;
    }
    m_player->setPlaylist(m_playlist);
    init();
    initDbus();

    m_playlist->setPlaybackMode(MMediaPlaylist::Loop);
//    m_playlist->setCurrentIndex(-1);
    connect(m_playlist,&MMediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    connect(m_player,&MMediaPlayer::stateChanged,this,&playController::slotStateChanged);
    connect(m_playlist,&MMediaPlaylist::playbackModeChanged,this,&playController::slotPlayModeChange);
//    connect(m_player,&MMediaPlayer::playErrorMsg,this,&playController::slotPlayErrorMsg);
    m_volume = volumeSetting->get("volume").toInt();
    delayMsecond(100);
    m_player->setVolume(m_volume);
}

void playController::init()
{
    volumeSetting = new QGSettings(LINGMOMUSIC);
    m_volume = volumeSetting->get("volume").toInt();
    delayMsecond(100);
    m_player->setVolume(m_volume);
    playSetting = new QGSettings(LINGMOMUSIC);
    m_playListName = playSetting->get("playlistname").toString();

    //如果保存的歌单是 歌曲列表 或者 我喜欢（我喜欢与歌曲列表必存在不做判断），那么保存的歌曲路径不做处理
    if(m_playListName == ALLMUSIC)
    {
        m_curList = ALLMUSIC;
    }
    else if(m_playListName == FAV)
    {
        m_curList = FAV;
    }
    else
    {
        int ret = g_db->checkPlayListExist(m_playListName);
        if(ret == DB_OP_SUCC)
        {
            if (m_playListName == SEARCH) {
                playSetting->set("playlistname", ALLMUSIC);
                m_curList = ALLMUSIC;
                playSetting->set("path", "");
            } else {
                m_curList = m_playListName;
                QString pat = playSetting->get("path").toString();
                int ref = g_db->checkIfSongExistsInPlayList(pat, m_curList);
                if(ref != DB_OP_SUCC)
                {
                    playSetting->set("path", "");
                }
            }
        }
        else
        {
            playSetting->set("playlistname", ALLMUSIC);
            m_curList = ALLMUSIC;
            playSetting->set("path", "");
        }
    }

    playModeSetting = new QGSettings(LINGMOMUSIC);
    m_mode = static_cast<PlayMode>(playModeSetting->get("playbackmode").toInt());

}

void playController::initDbus()
{
    QDBusConnection::sessionBus().connect(QString(), "/", "org.lingmo.media", "sinkVolumeChanged", this, SLOT(slotVolumeChange(QString,int,bool)));
}

int playController::getVolume()
{
    return m_volume;
}

void playController::delayMsecond(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void playController::delayMsecondSetVolume()
{
    int currentVolume = getVolume();
    KyInfo() << "currentVolume = " << currentVolume
             << "playState = " << getPlayer()->state();

    delayMsecond(100);
    setVolume(currentVolume);
}

void playController::setVolume(int volume)
{
    if(volume > 100) {
        volume = 100;
    }
    if(volume < 0) {
        volume = 0;
    }

    m_volume = volume;
    volumeSetting->set("volume", volume);

    KyInfo() << "m_receive = " << m_receive;
    if (!m_receive) {
        KyInfo() << "begin setVolume";
        m_player->setVolume(volume);
    } else {
        m_receive = false;
    }
}

void playController::onCurrentIndexChanged()
{
    qDebug() << "onCurrentIndexChanged";
}
void playController::onPositionChanged(double value)
{
    qDebug() << "onPositionChanged";
    if (m_player == nullptr) {
        return;
    }
    m_player->setPosition(m_player->duration() * value);
}
void playController::onNextSong()
{
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }
    m_playlist->next();
    m_player->play();
    curPlaylist();
    auto index = m_playlist->currentIndex();
    Q_EMIT curIndexChanged(index);
}
void playController::onPreviousSong()
{
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }
    m_playlist->previous();
    m_player->play();
    auto index = m_playlist->currentIndex();
    Q_EMIT curIndexChanged(index);
}

void playController::setCurList(QString renameList)
{
    m_curList = renameList;
}
void playController::onError()
{
    qDebug() << "onError";
}
void playController::onMediaStatusChanged()
{
    qDebug() << "onMediaStatusChanged";
}
playController::~playController(/* args */)
{
    if (m_playlist != nullptr) {
        m_playlist->deleteLater();
    }
    if (m_player != nullptr) {
        m_player->deleteLater();
    }
}

bool playController::playSingleSong(QString Path, bool isPlayNowOrNext)
{

}

void playController::slotStateChanged(MMediaPlayer::State newState)
{
    if(newState == MMediaPlayer::State::PlayingState)
        Q_EMIT playerStateChange(playController::PlayState::PLAY_STATE);
    else if(newState == MMediaPlayer::State::PausedState)
        Q_EMIT playerStateChange(playController::PlayState::PAUSED_STATE);
    else if(newState == MMediaPlayer::State::StoppedState)
        Q_EMIT playerStateChange(playController::PlayState::STOP_STATE);

    DbusAdapter *m_adapter = new DbusAdapter;
    QVariantMap changedProperties;
    changedProperties.insert("PlaybackStatus",QVariant::fromValue(QDBusVariant(m_playbackStatus)));
    m_adapter->notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    delete m_adapter;
}

void playController::slotPlayModeChange(MMediaPlaylist::PlaybackMode mode)
{
    if(mode == MMediaPlaylist::PlaybackMode::CurrentItemInLoop)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::CurrentItemInLoop));
    else if(mode == MMediaPlaylist::PlaybackMode::Sequential)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::Sequential));
    else if(mode == MMediaPlaylist::PlaybackMode::Loop)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::Loop));
    else if(mode == MMediaPlaylist::PlaybackMode::Random)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::Random));
}

void playController::slotIndexChange(int index)
{
    if(index == -1)
    {
        Q_EMIT signalNotPlaying();
        //当index == -1时，会调用positionChanged导致时长显示错误
        Q_EMIT singalChangePath("");
        Q_EMIT currentIndexAndCurrentList(-1,m_curList);
        playSetting->set("playlistname", m_curList);
        playSetting->set("path", "");
        return;
    }
    m_curIndex = index;
    MMediaContent content = m_playlist->media(index);
    QString path = content.canonicalUrl().toLocalFile();
    QFileInfo file(path.remove("file://"));
    if(file.exists())
    {
        x = 0;
        Q_EMIT currentIndexAndCurrentList(index,m_curList);
        Q_EMIT singalChangePath(path);
        if(m_curList == HISTORY)
        {
            playSetting->set("playlistname", ALLMUSIC);
        }
        else
        {
            playSetting->set("playlistname", m_curList);
        }
        playSetting->set("path", path);
    }
    else
    {
        x++;
        if(x > m_playlist->mediaCount())
        {
            x = 0;
//            index = -1;
//            m_playlist->setCurrentIndex(index);
            return;
        }
    }
}

//void playController::slotPlayErrorMsg(MMediaPlayer::ErrorMsg msg)
//{
////    m_msg = msg;
//    Q_EMIT playErrorMsg(msg);
//}

void playController::setPosition(int position)
{
    if (qAbs(m_player->position() - position) > 99)
       m_player->setPosition(position);
}

void playController::setPlayListName(QString playListName)
{
    playSetting->set("playlistname", playListName);
}

QString playController::getPlayListName()
{
    return playSetting->get("playlistname").toString();
}

QString playController::getPath()
{
    return playSetting->get("path").toString();
}

void playController::setMode(playController::PlayMode mode)
{
    m_mode = mode;
    playModeSetting->set("playbackmode", m_mode);
}

playController::PlayMode playController::mode() const
{
    return m_mode;
}

void playController::slotVolumeChange(QString app, int value, bool mute)
{
    if (app == "lingmo-music") {
        if (value < 0) {
            return;
        }

        if (value != m_volume) {
            KyInfo() << "value != m_volume."
                     << "value = " << value;

            m_receive = true;
            Q_EMIT signalVolume(value);
        }

        //mute = true静音  mute = false取消静音
        Q_EMIT signalMute(mute);
    }
}
