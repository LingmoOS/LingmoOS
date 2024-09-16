// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QTest>
#include <QTestEventList>
#include <QDebug>
#include <QAbstractButton>
#include <DSettingsDialog>
#include <dwidgetstype.h>

#include <unistd.h>
#include <gtest/gtest.h>

#include "application.h"
#include "dmr_settings.h"
#include "utils.h"
#include "movie_configuration.h"
#include "dbus_adpator.h"

using namespace dmr;
using namespace utils;

TEST(Settings, Settings)
{
    Settings::get().isSet(Settings::Flag::ClearWhenQuit);
    Settings::get().isSet(Settings::Flag::ShowThumbnailMode);
    Settings::get().isSet(Settings::Flag::AutoSearchSimilar);
    Settings::get().isSet(Settings::Flag::PreviewOnMouseover);
    Settings::get().isSet(Settings::Flag::MultipleInstance);
    Settings::get().isSet(Settings::Flag::PauseOnMinimize);
    Settings::get().settings()->sync();

    Settings::get().commonPlayableProtocols();
    Settings::get().commonPlayableProtocols();
    Settings::get().iscommonPlayableProtocol("dvb");
    Settings::get().screenshotLocation();
    Settings::get().screenshotNameTemplate();
    Settings::get().screenshotNameSeqTemplate();

    QString path("/usr/share/dde-introduction");
    Settings::get().setGeneralOption("last_open_path", path);
    QVariant v = Settings::get().generalOption("last_open_path");

    DSettingsOption settingsOption;
    emit settingsOption.valueChanged(v);

    DLineEdit edit;
    emit edit.editingFinished();
}

TEST(Settings, shortcut)
{
    Settings::get().settings()->setOption("shortcuts.play.enable", false);
    Settings::get().settings()->setOption("shortcuts.play.enable", true);
    Settings::get().settings()->setOption("shortcuts.play.playlist", "Shift+Return");
    Settings::get().settings()->setOption("shortcuts.play.movie_info", "Shift+Num+Enter");
    Settings::get().settings()->setOption("subtitle.font.size", 20);
    Settings::get().settings()->setOption("base.play.hwaccel", 1);
    Settings::get().settings()->setOption("base.decode.select", DecodeMode::AUTO);
    Settings::get().settings()->setOption("base.decode.select", DecodeMode::HARDWARE);
    Settings::get().settings()->setOption("base.decode.select", DecodeMode::SOFTWARE);
    emit Settings::get().hwaccelModeChanged("base.play.hwaccel", 1);

//    Settings::get().settings()->setOption("play.global_volume", 120);
}

TEST(Settings, mwDeconstruction)
{
    Platform_MainWindow *w = dApp->getMainWindow();
    w->close();
    delete w;
    w = nullptr;
}


