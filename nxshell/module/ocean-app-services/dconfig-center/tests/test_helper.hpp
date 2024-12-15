// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDir>

class EnvGuard {
public:
    void set(const char *name, const QByteArray &value)
    {
        m_name = name;
        m_originValue = qgetenv(m_name);
        qputenv(m_name, value);

        if (!QDir(value).exists()) {
            QDir().mkpath(value);
        }
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


class FileCopyGuard {
public:
    FileCopyGuard(const QString &source, const QString &target)
        : m_target(target)
    {
        if (!QFile::exists(QFileInfo(target).path()))
            QDir().mkpath(QFileInfo(target).path());
        QFile::copy(source, target);
    }
    ~FileCopyGuard(){ QFile::remove(m_target); }
private:
    QString m_target;
};
