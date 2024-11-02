#ifndef DBUSADAPTER_H
#define DBUSADAPTER_H

#include <QObject>
#include <QDBusConnection>
#include <QVariantMap>
//#include "UIControl/player/player.h"
#include "mainwidget.h"

class DbusAdapter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    // When this property changes, the org.freedesktop.DBus.Properties.PropertiesChanged signal is emitted with the new value.
    Q_PROPERTY(QString PlaybackStatus READ getPlaybackState)
    Q_PROPERTY(QString LoopStatus READ getLoopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ getRate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ getShuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ getMetadata)
    Q_PROPERTY(double Volume READ getVolume WRITE setVolume)
    Q_PROPERTY(double MinimumRate READ getMinimumRate)
    Q_PROPERTY(double MaximumRate READ getMaximumRate)
    Q_PROPERTY(bool CanGoNext READ getCanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ getCanGoPrevious)
    Q_PROPERTY(bool CanPlay READ getCanPlay)
    Q_PROPERTY(bool CanPause READ getCanPause)
    Q_PROPERTY(bool CanSeek READ getCanSeek)

    // When this property changes, the org.freedesktop.DBus.Properties.PropertiesChanged signal is NOT emitted with the new value.
    Q_PROPERTY(long long Position READ getPosition)
    Q_PROPERTY(bool CanControl READ getCanControl)


public:
    explicit DbusAdapter(QObject *parent = nullptr);
    void emitSeeked();

public:
    QString getPlaybackState();

    QString getLoopStatus();
    void setLoopStatus(QString s);

    double m_rate;
    double getRate();
    void setRate(double d);

    bool m_shuffle { false };
    bool getShuffle() { return m_shuffle; }
    void setShuffle(bool b) { m_shuffle = b; }

    QVariantMap m_metadata;
    QVariantMap getMetadata();
    void updateMetadata(const QVariantMap input);

    double getVolume();
    void setVolume(double b);

    long long getPosition();

    double m_minimumRate { 0.5 };
    double getMinimumRate() { return m_minimumRate; }

    double m_maximumRate { 3.0 };
    double getMaximumRate() { return m_maximumRate; }

    bool m_canGoNext { true };
    bool getCanGoNext() { return m_canGoNext; }

    bool m_canGoPrevious { true };
    bool getCanGoPrevious() { return m_canGoPrevious; }

    bool m_canPlay { true };
    bool getCanPlay() { return m_canPlay; }

    bool m_canPause { true };
    bool getCanPause() { return m_canPause; }

    bool m_canSeek { true };
    bool getCanSeek() { return m_canSeek; }

    bool m_canControl { true };
    bool getCanControl() { return m_canControl; }
    void setCanControl(const bool b) { m_canControl = b; }

    void initMetadata();
public Q_SLOTS:
    // mpris 使用
    void Stop() const;
    void Next() const;
    void Play() const;
    void Pause() const;
    void PlayPause();
    void Previous() const;
    void Seek(const qint64 Offset);
    void SetPosition(const QString& TrackId, const long long Position);
    void OpenUri(const QString& Uri);

    void VolumeUp() const;
    void VolumeDown() const;
    void FullScreen() const;
    void Exit() const;

    void notify(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

Q_SIGNALS:
    void Seeked(const qint64 url);
    void PropertiesChanged(const QString &interface_name, const QVariantMap &change_properties, const QStringList &invalidate_properties);

private:
    QString m_playbackStatus = "Stopped";
//    void notify(const QString &changedProperties, const QStringList &invalidatedProperties);
};

#endif // DBUSADAPTER_H
