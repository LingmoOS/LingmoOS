// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "dlayershellwindow.h"

#include <QObject>
#include <QWindow>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

class xcb_connection_t;
DS_BEGIN_NAMESPACE
class LayerShellEmulation : public QObject
{
    Q_OBJECT
public:
    explicit LayerShellEmulation(QWindow* window, QObject* parent = nullptr);

private slots:
    void onLayerChanged();
    // margins or anchor changed
    void onPositionChanged();
    void onExclusionZoneChanged();
    // void onKeyboardInteractivityChanged();

private:
    QWindow* m_window;
    DLayerShellWindow* m_dlayerShellWindow;
};
DS_END_NAMESPACE
