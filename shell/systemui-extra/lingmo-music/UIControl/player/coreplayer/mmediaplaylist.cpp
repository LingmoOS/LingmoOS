#include "mmediaplaylist.h"

MMediaPlaylist::MMediaPlaylist(QObject *parent)
    : QObject(parent)
{

}

QString MMediaPlaylist::getPlayFileName()
{
    //异常情况：播放列表为空
    if (m_playerList.isEmpty()) {
        return "";
    }
    //异常情况：当前播放的媒体在列表中的位置超过列表中媒体总数量
    if (m_index >= m_playerList.length()) {
        m_index = m_playerList.length();
        return m_playerList.last().toString();
    }
    return m_playerList.at(m_index).toString();
}

int MMediaPlaylist::currentIndex() const
{
    return m_index;
}

bool MMediaPlaylist::addMedia(const QUrl &items)
{
    bool isExists = QFileInfo::exists(items.toLocalFile());
    if (isExists) {
        // 单修改此处，会导致界面点击和实际播放不一致问题
//        m_playerList.append(items);
        notExistFileNumber++;
    }

    m_playerList.append(items);
    return true;
}

void MMediaPlaylist::next()
{
    //异常情况：播放列表为空
    if (m_playerList.isEmpty()) {
        return;
    }
    //异常情况：无当前播放媒体
    if (m_index < 0) {
        return;
    }
    switch (m_playbackMode) {
    case Random:
        m_index=randomIndex();
        //异常情况：无当前播放媒体
        if (m_index < 0) {
            return;
        }

        break;
    case Sequential:
        m_index++;
        if (m_index >= m_playerList.length()) {
           m_index = m_playerList.length() - 1;
        }
        break;
    default:
        m_index++;
        if (m_index >= m_playerList.length()) {
           m_index = 0;
        }
        break;
    }
    Q_EMIT currentIndexChanged(m_index);
    Q_EMIT stop();
}

void MMediaPlaylist::previous()
{
    //异常情况：播放列表为空
    if (m_playerList.isEmpty()) {
        return;
    }

    switch (m_playbackMode) {
    case Random:
        m_index=randomIndex();
        //异常情况：无当前播放媒体，或当前播放列表为空
        if (m_index < 0) {
            return;
        }
        break;
    case Sequential:
        m_index--;
        if (m_index < 0) {
            m_index = 0;
        }
        break;
    default:
        m_index--;
        if (m_index < 0) {
           m_index = m_playerList.length() - 1;
        }
        break;
    }
    Q_EMIT currentIndexChanged(m_index);
    Q_EMIT stop();
}

void MMediaPlaylist::setCurrentIndex(int index)
{
    //待设置的数量和设置之前一致则不处理，默认播放第一首除外
//    if (index == m_index && index != 0) {
//        return;
//    }
    //异常情况：要设置的媒体位置超过列表总长度
//    if (index >= m_playerList.length()) {
//        qDebug()<<"指定位置超过列表元素数量";
//        return;
//    }
    //统一所有非正常情况
    if (index < 0) {
        index = -1;
    }
    m_index = index;
    Q_EMIT currentIndexChanged(m_index);
}

void MMediaPlaylist::setPlaybackMode(MMediaPlaylist::PlaybackMode mode)
{
    //待设置的循环模式和设置之前一致则不处理
    if (mode == m_playbackMode) {
        return;
    }
    m_playbackMode = mode;
    Q_EMIT playbackModeChanged(mode);
}

int MMediaPlaylist::mediaCount() const
{
    return m_playerList.length();
}

MMediaContent MMediaPlaylist::media(int index) const
{
    //异常情况：要设置的媒体位置在列表中不存在
    if (index >= m_playerList.length() || index < 0) {
        return MMediaContent(QUrl(""));
    }
    return MMediaContent(m_playerList.at(index));
}

bool MMediaPlaylist::clear()
{
    m_playerList.clear();
    return true;
}

bool MMediaPlaylist::removeMedia(int pos)
{
    //异常情况：要移出的媒体位置在列表中不存在
    if (pos >= m_playerList.length() || pos < 0) {
        return false;
    }
    m_playerList.removeAt(pos);
    return true;
}

void MMediaPlaylist::playError()
{
    //当前仅在存在播放列表中的媒体本地文件被删除时触发
    //播放异常时，轮询所有列表中的媒体文件是否存在
    for (auto url : m_playerList) {
        //如果发现列表中有媒体文件没被删除
        if (QFileInfo::exists(url.toLocalFile())) {
            //如果是单曲循环则切换下一首
            if (m_playbackMode == CurrentItemInLoop) {
                next();
            }
            //按播放完成处理
            palyFinish();
            return;
        }
    }
    //列表中所有媒体的本地文件全部被删除了
    Q_EMIT currentIndexChanged(-1);
}

void MMediaPlaylist::playErrorMsg(int Damage)
{
    if (Damage == -2) {
        if (mediaCount() == 0) {
               return;
        }
        if (m_playerList.isEmpty()) {
               return;
           }
        if (m_index < 0) {
               return;
           }
        if (m_index >= m_playerList.length()) {
               m_index = 0;
           }
    }
}

void MMediaPlaylist::palyFinish()
{
    //如果没有待播放的媒体则不处理
    if (m_index < 0) {
        return;
    }

    //如果循环模式不是单曲循环则切换下一首
    if (m_playbackMode != CurrentItemInLoop) {
        next();
        Q_EMIT currentIndexChanged(m_index);
    }

    if(m_playbackMode == CurrentItemInLoop){
        if(!QFileInfo::exists(QUrl(getPlayFileName()).toLocalFile()))
            next();
    }

    Q_EMIT autoPlay(m_playbackMode);
}

MMediaPlaylist::PlaybackMode MMediaPlaylist::playbackMode() const
{
    return m_playbackMode;
}

int MMediaPlaylist::randomIndex()
{
    qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch());
    int lenPlaylist = m_playerList.length();
    if (lenPlaylist == 0) {
        // 播放列表沒有歌曲，此时不应有可用值
        return -1;
    }
    return qrand()%(m_playerList.length());
}


MMediaContent::MMediaContent(QUrl url)
{
    m_url = url;
}

QUrl MMediaContent::canonicalUrl() const
{
    return m_url;
}
