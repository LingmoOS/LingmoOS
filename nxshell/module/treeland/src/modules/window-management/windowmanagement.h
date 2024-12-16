// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <wserver.h>

#include <QQmlEngine>

struct treeland_window_management_v1;
WAYLIB_SERVER_USE_NAMESPACE

class WindowManagementV1 : public QObject, public Waylib::Server::WServerInterface
{
    Q_OBJECT
    Q_PROPERTY(DesktopState desktopState READ desktopState WRITE setDesktopState NOTIFY desktopStateChanged)

public:
    enum class DesktopState { Normal, Show, Preview };
    Q_ENUM(DesktopState)

    explicit WindowManagementV1(QObject *parent = nullptr);

    DesktopState desktopState();
    void setDesktopState(DesktopState state);

protected:
    void create(WServer *server) override;
    void destroy(WServer *server) override;
    wl_global *global() const override;

Q_SIGNALS:
    void requestShowDesktop(uint32_t state);
    void desktopStateChanged();

private:
    treeland_window_management_v1 *m_handle{ nullptr };
};
