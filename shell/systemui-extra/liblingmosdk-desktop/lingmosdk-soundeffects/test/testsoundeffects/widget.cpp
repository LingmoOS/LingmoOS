#include "widget.h"
#include <QStringList>
#include <QStringListModel>
#include <QComboBox>
#include <QDebug>
#include "ksoundeffects.h"
using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(600,500);
    QStringList  list;
    list << "SoundType::AUDIO_VOLUME_CHANGE"
         <<  "SoundType::BATTERY_LOW"
         <<  "SoundType::COMPLETE"
         <<  "SoundType::DEVICE_ADDED_FAILED"
         <<  "SoundType::DEVICE_ADDED"
         <<  "SoundType::DEVICE_REMOVED"
         <<  "SoundType::DIALOG_ERROR"
         <<  "SoundType::DIALOG_WARNING"
         <<  "SoundType::NOTIFICATION_GENERAL"
         <<  "SoundType::NOTIFICATION_SPECIAL"
         <<  "SoundType::OPERATION_FILE"
         <<  "SoundType::OPERATION_UNSUPPORTED"
         <<  "SoundType::POWER_PLUG"
         <<  "SoundType::SCREEN_CAPTURE"
         <<  "SoundType::SCREEN_RECORD"
         <<  "SoundType::START_UP"
         <<  "SoundType::TRASH_EMPTY";

    QStringListModel *model = new QStringListModel();
    model->setStringList(list);
    QComboBox* combox= new QComboBox(this);

    combox->setModel(model);

    connect(combox,&QComboBox::currentTextChanged,this,[=](QString text){
        if(text == "SoundType::AUDIO_VOLUME_CHANGE")
            kdk::KSoundEffects::playSound(SoundType::AUDIO_VOLUME_CHANGE);
        else if(text == "SoundType::BATTERY_LOW")
            kdk::KSoundEffects::playSound(SoundType::BATTERY_LOW);
        else if(text == "SoundType::COMPLETE")
            kdk::KSoundEffects::playSound(SoundType::COMPLETE);
        else if(text == "SoundType::DEVICE_ADDED_FAILED")
            kdk::KSoundEffects::playSound(SoundType::DEVICE_ADDED_FAILED);
        else if(text == "SoundType::DEVICE_ADDED")
            kdk::KSoundEffects::playSound(SoundType::DEVICE_ADDED);
        else if(text == "SoundType::DEVICE_REMOVED")
            kdk::KSoundEffects::playSound(SoundType::DEVICE_REMOVED);
        else if(text == "SoundType::DIALOG_ERROR")
            kdk::KSoundEffects::playSound(SoundType::DIALOG_ERROR);
        else if(text == "SoundType::DIALOG_WARNING")
            kdk::KSoundEffects::playSound(SoundType::DIALOG_WARNING);
        else if(text == "SoundType::NOTIFICATION_GENERAL")
            kdk::KSoundEffects::playSound(SoundType::NOTIFICATION_GENERAL);
        else if(text == "SoundType::NOTIFICATION_SPECIAL")
            kdk::KSoundEffects::playSound(SoundType::NOTIFICATION_SPECIAL);
        else if(text == "SoundType::OPERATION_FILE")
            kdk::KSoundEffects::playSound(SoundType::OPERATION_FILE);
        else if(text == "SoundType::OPERATION_UNSUPPORTED")
            kdk::KSoundEffects::playSound(SoundType::OPERATION_UNSUPPORTED);
        else if(text == "SoundType::POWER_PLUG")
            kdk::KSoundEffects::playSound(SoundType::POWER_PLUG);
        else if(text == "SoundType::SCREEN_CAPTURE")
            kdk::KSoundEffects::playSound(SoundType::SCREEN_CAPTURE);
        else if(text == "SoundType::SCREEN_RECORD")
            kdk::KSoundEffects::playSound(SoundType::SCREEN_RECORD);
        else if(text == "SoundType::START_UP")
            kdk::KSoundEffects::playSound(SoundType::START_UP);
        else if(text == "SoundType::TRASH_EMPTY")
            kdk::KSoundEffects::playSound(SoundType::TRASH_EMPTY);
    });
}

Widget::~Widget()
{
}

