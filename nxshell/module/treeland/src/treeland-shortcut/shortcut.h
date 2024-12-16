// Copyright (C) 2023 Dingyuan Zhang <zhangdingyuan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "qwayland-treeland-shortcut-manager-v1.h"

#include <QApplication>
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QtWaylandClient/QWaylandClientExtension>

class ShortcutContext;

class ShortcutV1 : public QWaylandClientExtensionTemplate<ShortcutV1>,
                        public QtWayland::treeland_shortcut_manager_v1
{
    Q_OBJECT
public:
    explicit ShortcutV1();

private:
    std::vector<ShortcutContext *> m_customShortcuts;
};

class ShortcutContext : public QWaylandClientExtensionTemplate<ShortcutContext>,
                        public QtWayland::treeland_shortcut_context_v1
{
    Q_OBJECT
public:
    explicit ShortcutContext(struct ::treeland_shortcut_context_v1 *object);
    ~ShortcutContext() override;

Q_SIGNALS:
    void shortcutHappended();

protected:
    void treeland_shortcut_context_v1_shortcut() override;
};

class Shortcut
{
public:
    Shortcut(const QString &path);

    virtual ~Shortcut() = default;

    void exec();

    QString shortcut();

private:
    QSettings m_settings;
};
