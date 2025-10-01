#pragma once
#include <QObject>
#include <QProcess>

class VolumeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted NOTIFY mutedChanged)

public:
    explicit VolumeController(QObject *parent = nullptr);

    Q_INVOKABLE int volume();
    Q_INVOKABLE bool muted();
    Q_INVOKABLE void updateVolume();

signals:
    void volumeChanged();
    void mutedChanged();

private:
    int m_volume;
    bool m_muted;
};