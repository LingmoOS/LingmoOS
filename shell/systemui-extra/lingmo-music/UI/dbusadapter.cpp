#include <QStringList>
#include "dbusadapter.h"

DbusAdapter::DbusAdapter(QObject *parent) : QObject(parent)
{
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.LingmoMusic");
    QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.LingmoMusic");
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", this, QDBusConnection::ExportAllContents);

    this->setProperty("CanControl", false);
//    initMetadata();
}

QString DbusAdapter::getPlaybackState()
{
    return Widget::mutual->getPlayState();
}

QString DbusAdapter::getLoopStatus()
{
    return Widget::mutual->getPlayMode();
}

void DbusAdapter::setLoopStatus(QString s)
{
    Widget::mutual->setPlayMode(s);
}

QVariantMap DbusAdapter::getMetadata()
{
    return Widget::mutual->getMetadata();
}

void DbusAdapter::updateMetadata(const QVariantMap input)
{
    bool needUpdate = false;

    for (auto iter = input.begin(); iter != input.end(); iter++) {
        ;
        if (m_metadata.find(iter.key()) != m_metadata.end() && m_metadata[iter.key()] != iter.value()) {
            m_metadata[iter.key()] = iter.value();
            needUpdate = true;
        }
    }
}

double DbusAdapter::getVolume()
{
    return double(Widget::mutual->getVolume()) / 100;
}

// mpris中volume的取值范围是[0,1]，但软件中取值范围是[0, 100]
void DbusAdapter::setVolume(double d)
{
    if (d < 0) {
        d = 0;
    } else if (d > 1) {
        qWarning() << "setVolume error, bigger than 1, volume is :" << d;
        return;
    }

    Widget::mutual->setVolume(d*100);

    QVariantMap changedProperties;
    changedProperties.insert("Volume",QVariant::fromValue(QDBusVariant(d)));
    notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
}

long long DbusAdapter::getPosition()
{
    return Widget::mutual->getPosition() * 1000;
}

void DbusAdapter::initMetadata()
{
    // 表示路径
    m_metadata.insert("mpris:trackid", 0);
    m_metadata.insert("mpris:length", 0);
    m_metadata.insert("mpris:artUrl", "");
    m_metadata.insert("xesam:album", "");
    m_metadata.insert("xesam:albumArtist", "");
    m_metadata.insert("xesam:artist", "");
    m_metadata.insert("xesam:asText", "");
    m_metadata.insert("xesam:audioBPM", 0);
    m_metadata.insert("xesam:autoRating", 0.0);
    QStringList commont;
    m_metadata.insert("xesam:comment", commont);
    QStringList composer;
    m_metadata.insert("xesam:composer", composer);
    m_metadata.insert("xesam:contentCreated", "");
    m_metadata.insert("xesam:discNumber", 0);
    m_metadata.insert("xesam:firstUsed", "");
    QStringList genre;
    m_metadata.insert("xesam:genre", genre);
    m_metadata.insert("xesam:lastUsed", "");
    QStringList lyricist;
    m_metadata.insert("xesam:lyricist", lyricist);
    m_metadata.insert("xesam:title", "");
    m_metadata.insert("xesam:trackNumber", 0);
    m_metadata.insert("xesam:url", "");
    m_metadata.insert("xesam:useCount", 0);
    m_metadata.insert("xesam:userRating", 0.0);
}

void DbusAdapter::Stop() const
{
    Widget::mutual->Stop();
}

void DbusAdapter::Next() const
{
    Widget::mutual->Next();
}

void DbusAdapter::Play() const
{
    Widget::mutual->Play();
}

void DbusAdapter::Pause() const
{
    Widget::mutual->Pause();
}

void DbusAdapter::PlayPause()
{
    qDebug() << "into PlayPause throught mpris.";
    Widget::mutual->PlayPause();
    QVariantMap changedProperties;
    changedProperties.insert("PlaybackStatus",QVariant::fromValue(QDBusVariant(m_playbackStatus)));
    notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
}

void DbusAdapter::Previous() const
{
    Widget::mutual->Previous();
}

void DbusAdapter::VolumeUp() const
{
    Widget::mutual->VolumeUp();
}

void DbusAdapter::VolumeDown() const
{
    Widget::mutual->VolumeDown();
}

void DbusAdapter::FullScreen() const
{
    Widget::mutual->slotShowMaximized();
}

void DbusAdapter::Exit() const
{
    Widget::mutual->slotClose();
}

/**
 * @brief DbusAdapter::Seek
 * @param offset 偏移微妙数，正数表示向后，负数表示向前
 * 如果结果小于0，表示从开始位置播放；如果结果大于媒体时长，表示播放下一曲
 */
void DbusAdapter::Seek(const qint64 Offset)
{
    qDebug() << "seek :" << Offset;
    // 如果小于0.1ms就不进行操作
    if (qAbs(Offset) > 1000*99) {
        Widget::mutual->Seek(Offset / 1000);
    }

}

void DbusAdapter::SetPosition(const QString &TrackId, const qint64 Position)
{

}

void DbusAdapter::OpenUri(const QString &Uri)
{

}

void DbusAdapter::notify(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString objectPath = "/org/mpris/MediaPlayer2";
    QDBusMessage signalMessage= QDBusMessage::createSignal(objectPath,"org.freedesktop.DBus.Properties","PropertiesChanged");
//    QVariantMap changed;
//    changed.insert("PlaybackStatus",QVariant(PlaybackStatus));
    signalMessage << interfaceName << changedProperties << invalidatedProperties;
    connection.send(signalMessage);
}

double DbusAdapter::getRate()
{
    qDebug() << "getRate";
    return Widget::mutual->getPlayRate();
}


void DbusAdapter::setRate(double d)
{
    qDebug() << "setRate:" << d;
    if (d >= m_minimumRate && d <= m_maximumRate) {
        Widget::mutual->setPlayRate(d);
    } else {
        qWarning() << "dbus set rate error, rate is :" << d;
    }
    QVariantMap changedProperties;
    changedProperties.insert("Rate",QVariant::fromValue(QDBusVariant(d)));
    notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
}
