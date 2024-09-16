// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QTest>
#include <QTestEventList>
#include <QDebug>
#include <QTimer>
#include <QAbstractButton>
#include <DSettingsDialog>
#include <dwidgetstype.h>
#include <QtCore/QMetaObject>
#include <QGuiApplication>
#include <QWidget>

#include <unistd.h>
#include <gtest/gtest.h>

#define protected public
#define private public
#include "src/common/mainwindow.h"
#undef protected
#undef private
#include "application.h"
#include "src/libdmr/player_engine.h"
#include "src/widgets/toolbox_proxy.h"
#include "src/widgets/toolbutton.h"
#include "src/widgets/playlist_widget.h"
#include "src/widgets/slider.h"
#include "src/widgets/movieinfo_dialog.h"
#include "src/widgets/url_dialog.h"
#include "src/widgets/dmr_lineedit.h"
#include "src/common/actions.h"
#include "src/backends/mpv/mpv_glwidget.h"
#include "utils.h"
#include "actions.h"
#include "dbus_adpator.h"
#include "dbusutils.h"
#include "burst_screenshots_dialog.h"
#include "mpv_proxy.h"

TEST(MainWindow, init)
{
    Window *w = dApp->getMainWindow();

    QTest::qWait(100);
}
