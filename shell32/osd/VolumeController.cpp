#include "VolumeController.h"
#include <QRegularExpression>

VolumeController::VolumeController(QObject *parent) : QObject(parent), m_volume(0), m_muted(false)
{
    updateVolume();
}

int VolumeController::volume() { return m_volume; }
bool VolumeController::muted() { return m_muted; }

void VolumeController::updateVolume()
{
    // 获取音量
    QProcess p;
    p.start("pactl", {"get-sink-volume", "@DEFAULT_SINK@"});
    p.waitForFinished();
    QString out = p.readAllStandardOutput();
    QRegularExpression re("(\\d+)%");
    auto match = re.match(out);
    int vol = match.hasMatch() ? match.captured(1).toInt() : 0;
    if (vol != m_volume) { m_volume = vol; emit volumeChanged(); }

    // 获取静音状态
    p.start("pactl", {"get-sink-mute", "@DEFAULT_SINK@"});
    p.waitForFinished();
    QString out2 = p.readAllStandardOutput();
    bool mutedNow = out2.contains("yes");
    if (mutedNow != m_muted) { m_muted = mutedNow; emit mutedChanged(); }
}