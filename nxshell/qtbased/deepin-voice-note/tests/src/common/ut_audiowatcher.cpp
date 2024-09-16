// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_audiowatcher.h"
#include "audiowatcher.h"
#include "stub.h"
#include <QAction>

QVariant stub_settings_value(QString value)
{
    Q_UNUSED(value);
    return QVariant();
}

UT_AudioWatcher::UT_AudioWatcher()
{
}

void UT_AudioWatcher::SetUp()
{
    m_AudioWatcher = new AudioWatcher;
}

void UT_AudioWatcher::TearDown()
{
    delete m_AudioWatcher;
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_initWatcherCofing_001)
{
    m_AudioWatcher->initWatcherCofing();
    EXPECT_TRUE(m_AudioWatcher->m_fNeedDeviceChecker);

    Stub stub;
    stub.set(ADDR(QSettings, value), stub_settings_value);
    AudioWatcher audioWatcher;
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_initConnections_001)
{
    m_AudioWatcher->initConnections();
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_initDeviceWacther_001)
{
    m_AudioWatcher->initDeviceWacther();
    EXPECT_FALSE(m_AudioWatcher->m_pAudioInterface.isNull()) << "m_pAudioInterface";
    EXPECT_FALSE(m_AudioWatcher->m_pDefaultSink.isNull()) << "m_pDefaultSink";
    EXPECT_FALSE(m_AudioWatcher->m_pDefaultSource.isNull()) << "m_pDefaultSource";
    EXPECT_EQ(m_AudioWatcher->m_inAudioPortVolume, m_AudioWatcher->m_pDefaultSource->volume())
        << "m_inAudioPortVolume";

    EXPECT_EQ(m_AudioWatcher->m_inAudioPort, m_AudioWatcher->m_pDefaultSource->activePort())
        << "m_inAudioPort";

    EXPECT_EQ(m_AudioWatcher->m_inAudioMute, m_AudioWatcher->m_pDefaultSource->mute())
        << "m_inAudioMute";

    EXPECT_EQ(m_AudioWatcher->m_outAudioPortVolume, m_AudioWatcher->m_pDefaultSink->volume())
        << "m_outAudioPortVolume";

    EXPECT_EQ(m_AudioWatcher->m_outAudioPort, m_AudioWatcher->m_pDefaultSink->activePort())
        << "m_outAudioPort";

    EXPECT_EQ(m_AudioWatcher->m_outAudioMute, m_AudioWatcher->m_pDefaultSink->mute())
        << "m_outAudioMute";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_getDeviceName_001)
{
    m_AudioWatcher->m_fNeedDeviceChecker = true;
    m_AudioWatcher->m_outAudioPort.availability = 0;
    QString device = m_AudioWatcher->getDeviceName(AudioWatcher::Internal);
    EXPECT_TRUE(device.endsWith(".monitor") || device.isEmpty())
        << "AudioMode = Internal, m_outAudioPort.availability = 0";

    m_AudioWatcher->m_inAudioPort.availability = 0;
    EXPECT_FALSE(m_AudioWatcher->getDeviceName(AudioWatcher::Micphone).endsWith(".monitor"))
        << "AudioMode = Micphone, m_inAudioPort.availability = 0";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_getDeviceName_002)
{
    m_AudioWatcher->m_fNeedDeviceChecker = true;
    m_AudioWatcher->m_outAudioPort.availability = 2;
    QString device = m_AudioWatcher->getDeviceName(AudioWatcher::Internal);
    EXPECT_TRUE(device.endsWith(".monitor") || device.isEmpty())
        << "AudioMode = Internal, m_outAudioPort.availability = 2";

    m_AudioWatcher->m_inAudioPort.availability = 2;
    EXPECT_FALSE(m_AudioWatcher->getDeviceName(AudioWatcher::Micphone).endsWith(".monitor"))
        << "AudioMode = Micphone, m_inAudioPort.availability = 2";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_getDeviceName_003)
{
    m_AudioWatcher->m_fNeedDeviceChecker = false;
    QString device = m_AudioWatcher->getDeviceName(AudioWatcher::Internal);
    EXPECT_TRUE(device.endsWith(".monitor") || device.isEmpty())
        << "AudioMode = Internal, m_fNeedDeviceChecker = false";

    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->name(), m_AudioWatcher->getDeviceName(AudioWatcher::Micphone))
        << "AudioMode = Micphone, m_fNeedDeviceChecker = false";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_getDeviceName_004)
{
    m_AudioWatcher->m_fNeedDeviceChecker = true;
    m_AudioWatcher->m_outAudioPort.availability = 1;
    EXPECT_TRUE(m_AudioWatcher->getDeviceName(AudioWatcher::Internal).isEmpty())
        << "AudioMode = Internal, m_fNeedDeviceChecker = true, "
           "m_inAudioPort.availability = 1";

    m_AudioWatcher->m_inAudioPort.availability = 1;
    EXPECT_TRUE(m_AudioWatcher->getDeviceName(AudioWatcher::Micphone).isEmpty())
        << "AudioMode = Micphone, m_fNeedDeviceChecker = true, "
           "m_inAudioPort.availability = 1";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_getVolume_001)
{
    m_AudioWatcher->m_outAudioPortVolume = 1;
    m_AudioWatcher->m_inAudioPortVolume = 2;

    EXPECT_EQ(m_AudioWatcher->m_outAudioPortVolume, m_AudioWatcher->getVolume(AudioWatcher::Internal))
        << "AudioMode = Internal";
    EXPECT_EQ(m_AudioWatcher->m_inAudioPortVolume, m_AudioWatcher->getVolume(AudioWatcher::Micphone))
        << "AudioMode = Micphone";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_getMute_001)
{
    m_AudioWatcher->m_inAudioMute = true;
    m_AudioWatcher->m_outAudioMute = false;

    EXPECT_EQ(m_AudioWatcher->m_outAudioMute, m_AudioWatcher->getMute(AudioWatcher::Internal))
        << "AudioMode = Internal";
    EXPECT_EQ(m_AudioWatcher->m_inAudioMute, m_AudioWatcher->getMute(AudioWatcher::Micphone))
        << "AudioMode = Micphone";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onDefaultSourceActivePortChanged_001)
{
    AudioPort ap;
    ap.name = "AudioPort";
    m_AudioWatcher->onDefaultSourceActivePortChanged(ap);
    EXPECT_EQ(m_AudioWatcher->m_inAudioPort, ap);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onDefaultSinkActivePortChanged_001)
{
    AudioPort ap;
    ap.name = "AudioPort";
    m_AudioWatcher->onDefaultSinkActivePortChanged(ap);
    EXPECT_EQ(m_AudioWatcher->m_outAudioPort, ap);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onDefaultSourceChanaged_001)
{
    AudioPort tmpAudioPort = m_AudioWatcher->m_pDefaultSource->activePort();
    QDBusObjectPath value("/com/deepin/daemon/Audio/Source2");
    m_AudioWatcher->onDefaultSourceChanaged(value);
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->volume(), m_AudioWatcher->m_inAudioPortVolume);
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->mute(), m_AudioWatcher->m_inAudioMute);
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->activePort(), m_AudioWatcher->m_inAudioPort);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onDefaultSinkChanaged_001)
{
    AudioPort tmpAudioPort = m_AudioWatcher->m_pDefaultSink->activePort();
    QDBusObjectPath value("/com/deepin/daemon/Audio/Sink2");
    m_AudioWatcher->onDefaultSinkChanaged(value);
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->volume(), m_AudioWatcher->m_inAudioPortVolume);
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->mute(), m_AudioWatcher->m_inAudioMute);
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->activePort(), m_AudioWatcher->m_inAudioPort);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSourceVolumeChanged_001)
{
    double tmpdl = 0.8;
    m_AudioWatcher->m_inAudioPort.name = m_AudioWatcher->m_pDefaultSource->activePort().name;
    m_AudioWatcher->m_inAudioPort.description = "m_inAudioPort";
    m_AudioWatcher->m_pDefaultSource->activePort().description = "activePort";
    m_AudioWatcher->onSourceVolumeChanged(tmpdl);
    EXPECT_EQ(0.8, m_AudioWatcher->m_inAudioPortVolume) << "value = 0.8";
    EXPECT_NE(m_AudioWatcher->m_pDefaultSource->activePort(), m_AudioWatcher->m_inAudioPort);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSourceVolumeChanged_002)
{
    double tmpdl = 0.6;
    m_AudioWatcher->m_inAudioPort.name = "activePort_2";
    m_AudioWatcher->m_pDefaultSource->activePort().name = "activePort";
    m_AudioWatcher->onSourceVolumeChanged(tmpdl);
    EXPECT_EQ(0.6, m_AudioWatcher->m_inAudioPortVolume) << "value = 0.6";
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSource->activePort(), m_AudioWatcher->m_inAudioPort);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSinkVolumeChanged_001)
{
    double tmpdl = 0.8;
    m_AudioWatcher->m_outAudioPort.name = m_AudioWatcher->m_pDefaultSink->activePort().name;
    m_AudioWatcher->m_outAudioPort.description = "m_outAudioPort";
    m_AudioWatcher->m_pDefaultSink->activePort().description = "activePort";
    m_AudioWatcher->onSinkVolumeChanged(tmpdl);
    EXPECT_EQ(0.8, m_AudioWatcher->m_outAudioPortVolume) << "value = 0.8";
    EXPECT_NE(m_AudioWatcher->m_pDefaultSink->activePort(), m_AudioWatcher->m_outAudioPort);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSinkVolumeChanged_002)
{
    double tmpdl = 0.6;
    m_AudioWatcher->m_outAudioPort.name = "m_outAudioPort";
    m_AudioWatcher->m_pDefaultSink->activePort().name = "activePort";
    m_AudioWatcher->onSinkVolumeChanged(tmpdl);
    EXPECT_EQ(0.6, m_AudioWatcher->m_outAudioPortVolume) << "value = 0.8";
    EXPECT_EQ(m_AudioWatcher->m_pDefaultSink->activePort(), m_AudioWatcher->m_outAudioPort);
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSourceMuteChanged_001)
{
    m_AudioWatcher->m_inAudioMute = false;
    m_AudioWatcher->onSourceMuteChanged(true);
    EXPECT_EQ(true, m_AudioWatcher->m_inAudioMute) << "value = true";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSourceMuteChanged_002)
{
    m_AudioWatcher->m_inAudioMute = true;
    m_AudioWatcher->onSourceMuteChanged(false);
    EXPECT_EQ(false, m_AudioWatcher->m_inAudioMute) << "value = false";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSinkMuteChanged_001)
{
    m_AudioWatcher->m_outAudioMute = false;
    m_AudioWatcher->onSinkMuteChanged(true);
    EXPECT_EQ(true, m_AudioWatcher->m_outAudioMute) << "value = true";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_onSinkMuteChanged_002)
{
    m_AudioWatcher->m_outAudioMute = true;
    m_AudioWatcher->onSinkMuteChanged(false);
    EXPECT_EQ(false, m_AudioWatcher->m_outAudioMute) << "value = false";
}

TEST_F(UT_AudioWatcher, UT_AudioWatcher_enumtest_001)
{
    QAction *tmpact = new QAction("test");
    QMetaEnum metaEnum = QMetaEnum::fromType<AudioWatcher::AudioMode>();
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        AudioWatcher::AudioMode temp = static_cast<AudioWatcher::AudioMode>(metaEnum.value(i));
        QString key = QString("test").append(i);
        tmpact->setProperty(qPrintable(key), temp);
        AudioWatcher::AudioMode tem = tmpact->property(qPrintable(key)).value<AudioWatcher::AudioMode>();
        EXPECT_EQ(temp, tem) << "temp is: " << temp << " and tem is: " << tem;
    }
    delete tmpact;
}
