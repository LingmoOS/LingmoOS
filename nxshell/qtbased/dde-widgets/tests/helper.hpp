// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtGlobal>

static const char *ExamplePluginId = "org.deepin.dde.widgets.ExampleWidget";

class EnvGuard {
public:
    void set(const char *name, const QByteArray &value)
    {
        m_name = name;
        m_originValue = qgetenv(m_name);
        qputenv(m_name, value);
    }
    void restore()
    {
        qputenv(m_name, m_originValue);
    }
    QString value()
    {
        return qgetenv(m_name);
    }
private:
    QByteArray m_originValue;
    const char* m_name = nullptr;
};

struct PluginGuard {
    PluginGuard() {
        static const char *ExamplePluginPath = "../app/plugins";
        guard.set("DDE_WIDGETS_PLUGIN_DIRS", ExamplePluginPath);
    }
    ~PluginGuard() {
        guard.restore();
    }
    EnvGuard guard;
};
