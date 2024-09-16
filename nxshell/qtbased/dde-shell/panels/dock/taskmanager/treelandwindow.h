// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstractwindow.h"
#include "qwayland-treeland-foreign-toplevel-manager-v1.h"

#include <cstdint>
#include <sys/types.h>

#include <QScopedPointer>
#include <QtWaylandClient/QWaylandClientExtension>

namespace dock {
class ForeignToplevelHandle : public QWaylandClientExtensionTemplate<ForeignToplevelHandle>, public QtWayland::ztreeland_foreign_toplevel_handle_v1
{
    Q_OBJECT

public:
    explicit ForeignToplevelHandle(struct ::ztreeland_foreign_toplevel_handle_v1 *object);
    bool isReady() const;
    uint32_t id() const;
    pid_t pid() const;
    QString title() const;
    QList<uint32_t> state() const;

Q_SIGNALS:
    // those signals only used for windowmonitor
    void handlerIsReady();
    void handlerIsDeleted();

    // for window info
    void pidChanged();
    void titleChanged();
    void isActiveChanged();

protected:
    void ztreeland_foreign_toplevel_handle_v1_pid(uint32_t pid) override;
    void ztreeland_foreign_toplevel_handle_v1_title(const QString &title) override;
    void ztreeland_foreign_toplevel_handle_v1_app_id(const QString &app_id) override;
    void ztreeland_foreign_toplevel_handle_v1_identifier(uint32_t identifier) override;
    void ztreeland_foreign_toplevel_handle_v1_state(wl_array *state) override;
    void ztreeland_foreign_toplevel_handle_v1_done() override;
    void ztreeland_foreign_toplevel_handle_v1_closed() override;

private:
    uint32_t m_pid;
    QString m_title;
    bool m_isReady;
    QString m_appId;
    uint32_t m_identifier;

    QList<uint32_t> m_states;
};

class TreeLandWindowMonitor;

class TreeLandWindow : public AbstractWindow
{
    Q_OBJECT

    enum WindowState {
        Active      = QtWayland::ztreeland_foreign_toplevel_handle_v1::state_activated,
        Maximized   = QtWayland::ztreeland_foreign_toplevel_handle_v1::state_maximized,
        Minimized   = QtWayland::ztreeland_foreign_toplevel_handle_v1::state_minimized,
        Fullscreen  = QtWayland::ztreeland_foreign_toplevel_handle_v1::state_fullscreen
    };

public:
    ~TreeLandWindow();

    uint32_t id() override;
    pid_t pid() override;
    QString icon() override;
    QString title() override;
    bool isActive() override;
    bool shouldSkip() override;
    bool isMinimized() override;
    bool allowClose() override;
    bool isAttention() override;

    void close() override;
    void activate() override;
    void maxmize() override;
    void minimize() override;
    void killClient() override;
    void setWindowIconGeometry(const QWindow* baseWindow, const QRect& gemeotry) override;

private:
    void updatePid() override;
    void updateIcon() override;
    void updateTitle() override;
    void updateIsActive() override;
    void updateShouldSkip() override;
    void updateAllowClose() override;
    void updateIsMinimized() override;

    void setForeignToplevelHandle(ForeignToplevelHandle* handle);

    bool isReady();

private:
    friend class TreeLandWindowMonitor;

    TreeLandWindow(uint32_t id, QObject *parent = nullptr);

private:
    uint32_t m_id;

    QScopedPointer<ForeignToplevelHandle> m_foreignToplevelHandle;
};
}
