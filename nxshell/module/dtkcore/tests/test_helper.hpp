// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDir>

class EnvGuard {
public:
    void set(const char *name, const QByteArray &value, bool mkpath = true)
    {
        m_name = name;
        if (m_originValue.isEmpty())
            m_originValue = qgetenv(m_name);
        qputenv(m_name, value);

        if (mkpath && !QDir(value).exists()) {
            QDir().mkpath(value);
        }
    }
    void unset(const char *name)
    {
        m_name = name;
        m_originValue = qgetenv(m_name);
        qunsetenv(m_name);
    }
    void restore()
    {
        qputenv(m_name, m_originValue);
    }
    QString value()
    {
        return qgetenv(m_name);
    }
    ~EnvGuard() {
        if (m_name)
            restore();
    }
private:
    QByteArray m_originValue;
    const char* m_name = nullptr;
};

class FileGuard {
public:
    explicit FileGuard(const QString &fileName)
        : m_fileName(fileName){ }
    virtual ~FileGuard() {
        QFile::remove(m_fileName);
    }
    const QString fileName() const { return m_fileName; }
private:
    QString m_fileName;
};

class FileCopyGuard : public FileGuard {
public:
    FileCopyGuard(const QString &source, const QString &target)
        : FileGuard(target)
    {
        if (!QFile::exists(QFileInfo(target).path()))
            QDir().mkpath(QFileInfo(target).path());
        QFile::copy(source, target);
    }
};
