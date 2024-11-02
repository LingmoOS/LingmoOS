#include "ksoundeffects.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QMediaPlayer>
#include <QDebug>
#include <QGSettings/QGSettings>

namespace kdk {

static  QGSettings* g_gsetting=nullptr;
static  QGSettings* g_styleGsetting=nullptr;

static QString audioFileFields(SoundType type)
{
    QString str;
    switch (type)
    {
        case AUDIO_VOLUME_CHANGE:
            str="audio-volume-change";
            break;
        case BATTERY_LOW:
            str="battery-low";
            break;
        case COMPLETE:
            str="complete";
            break;
        case DEVICE_ADDED_FAILED:
            str="device-added-failed";
            break;
        case DEVICE_ADDED:
            str="device-added";
            break;
        case DEVICE_REMOVED:
            str="device-removed";
            break;
        case DIALOG_ERROR:
            str="dialog-error";
            break;
        case DIALOG_WARNING:
            str="dialog-warning";
            break;
        case NOTIFICATION_GENERAL:
            str="notification-general";
            break;
        case NOTIFICATION_SPECIAL:
            str="notification-special";
            break;
        case OPERATION_FILE:
            str="operation-file";
            break;
        case OPERATION_UNSUPPORTED:
            str="operation-unsupported";
            break;
        case POWER_PLUG:
            str="power-plug";
            break;
        case SCREEN_CAPTURE:
            str="screen-capture";
            break;
        case SCREEN_RECORD:
            str="screen-record";
            break;
        case START_UP:
            str="start-up";
            break;
        case TRASH_EMPTY:
            str="trash-empty";
            break;
        default:
            break;
    }
    return str;
}

static void themeSoundPlay(SoundType type,QMediaPlayer* player,QString str,QString soundPath)
{
    if(g_gsetting->keys().contains("themeName"))
    {
        //初始化样式主题
        QString widgetThemeName = g_gsetting->get("themeName").toString();
        if(widgetThemeName == "Light-Seeking")
        {
            qDebug()<<"Light-Seeking";
            soundPath=str.arg("Light-Seeking").arg(audioFileFields(type));
        }
        else if(widgetThemeName == "HeYin")
        {
            qDebug()<<"HeYin";
            soundPath=str.arg("HeYin").arg(audioFileFields(type));
        }
        player->setMedia(QUrl::fromLocalFile(soundPath));
        if(player->error()==QMediaPlayer::NoError)
            player->play();
        else
            qWarning()<<"Description Failed to play the audio file!";
        return;
    }
    else if(g_styleGsetting->keys().contains("widgetThemeName"))
    {
        QString widgetThemeName = g_gsetting->get("widget-theme-name").toString();
        if(widgetThemeName == "default")
        {
            qDebug()<<"default";
            soundPath=str.arg("Light-Seeking").arg(audioFileFields(type));
        }
        else if(widgetThemeName == "fashion")
        {
            qDebug()<<"fashion";
            soundPath=str.arg("HeYin").arg(audioFileFields(type));
        }
        player->setMedia(QUrl::fromLocalFile(soundPath));
        if(player->error()==QMediaPlayer::NoError)
            player->play();
        else
            qWarning()<<"Description Failed to play the audio file!";
        return;
    }
    else
        qWarning()<<"No sound theme or system theme name!";
    return;
}
static void playAudioSound(SoundType type)
{
    QMediaPlayer* player = new QMediaPlayer;
    QString str("/usr/share/sounds/%1/stereo/%2.ogg");
    QString soundPath;
    if(!g_gsetting)
    {
        if(QGSettings::isSchemaInstalled("org.lingmo.style"))
        {
            g_styleGsetting = new QGSettings("org.lingmo.style");
            if(!g_styleGsetting)
                //没有主题 可能是V10
                return;
            else
            {
                if(QGSettings::isSchemaInstalled("org.lingmo.sound"))
                {
                    g_gsetting = new QGSettings("org.lingmo.sound");
                    if(g_gsetting->get("event-sounds").toBool())
                        themeSoundPlay(type,player,str,soundPath);
                    else
                        qWarning()<<"The system tips sound is not enabled";
                }
            }
        }
    }
    else
    {
        if(g_gsetting->get("event-sounds").toBool())
            themeSoundPlay(type,player,str,soundPath);
        else
            qWarning()<<"The system tips sound is not enabled";
    }
}

KSoundEffects::KSoundEffects()
{
}

void KSoundEffects::playSound(SoundType type)
{
    QDBusInterface interface("org.lingmo.sound.theme.player",
                            "/org/lingmo/sound/theme/player",
                            "org.lingmo.sound.theme.player",
                            QDBusConnection::sessionBus());

    if(!interface.isValid())
    {
        qDebug() << QDBusConnection::sessionBus().lastError().message()<<QString("Try to play the audio file");
        playAudioSound(type);
        return ;
    }

    QDBusReply<bool> reply = interface.call("playAlertSound",audioFileFields(type));

    if(reply.isValid())
    {
        bool value = reply.value();
        if(value)
            qDebug() << QString("Client play alert sound = %1").arg(value);
        else
        {
            qDebug() << QString("Client play alert sound = %1 ,Try to play the audio file").arg(value);
            playAudioSound(type);
        }
        return ;
    }
    else
    {
        qDebug() << "Play alert sound method call failed ! Try to play the audio file";
        playAudioSound(type);
        return ;
    }
    return ;
}

}
