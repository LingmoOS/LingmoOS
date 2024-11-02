#ifndef KSOUNDEFFECTS_H
#define KSOUNDEFFECTS_H

#include "lingmosdk-soundeffects_global.h"
#include <QObject>

namespace kdk
{
/**
 * @defgroup NotificationModule
 */

/**
 * @brief 声音类型，不同场景下可以选择不同枚举类型来播放不同音效
 */

enum SoundType{
    AUDIO_VOLUME_CHANGE,
    BATTERY_LOW,
    COMPLETE,
    DEVICE_ADDED_FAILED,
    DEVICE_REMOVED,
    DEVICE_ADDED,
    DIALOG_ERROR,
    DIALOG_WARNING,
    NOTIFICATION_GENERAL,
    NOTIFICATION_SPECIAL,
    OPERATION_FILE,
    OPERATION_UNSUPPORTED,
    POWER_PLUG,
    SCREEN_CAPTURE,
    SCREEN_RECORD,
    START_UP,
    TRASH_EMPTY
};

/**
 * @brief 音效通知，可以根据输入的类型触发不同的音效提示用户
 */
class LINGMOSDKSOUNDEFFECTS_EXPORT KSoundEffects : public QObject
{
    Q_OBJECT
public:
    KSoundEffects();

    /**
     * @brief 播放系统音效
     * @param soundEvent
     * @return
     */
    static void playSound(SoundType type);

};
}
#endif // KSOUNDEFFECTS_H
