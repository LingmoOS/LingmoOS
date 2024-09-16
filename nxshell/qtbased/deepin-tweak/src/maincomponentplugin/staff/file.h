// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <memory>
#include <optional>
#include <QByteArray>
#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QtQml>

class FileMode : public QObject
{
    Q_OBJECT
    Q_ENUMS(Mode);

public:
    enum Mode {
        NotOpen = QFile::NotOpen,
        ReadOnly = QFile::ReadOnly,
        WriteOnly = QFile::WriteOnly,
        ReadWrite = ReadOnly | WriteOnly,
        Append = QFile::Append,
        Truncate = QFile::Truncate,
        Text = QFile::Text,
        Unbuffered = QFile::Unbuffered,
        NewOnly = QFile::NewOnly,
        ExistingOnly = QFile::ExistingOnly
    };
};

class FileBackend : public QObject
{
    Q_OBJECT
public:
    FileBackend(const QString &file, QObject *parent = nullptr)
        : QObject(parent)
        , file(new QFile(file))
    {}

    ~FileBackend() override {}

    Q_INVOKABLE bool open(const QVariant &mode);
    Q_INVOKABLE void close();
    Q_INVOKABLE QByteArray read(qint64 maxSize);
    Q_INVOKABLE QByteArray readAll();
    Q_INVOKABLE qint64 write(const QByteArray &data);
    Q_INVOKABLE bool rename(const QString &newName);
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool flush();
    Q_INVOKABLE bool exists() const;
    Q_INVOKABLE bool seek(qint64 pos);
    Q_INVOKABLE qint64 pos() const;
    Q_INVOKABLE qint64 size() const;

private:
    std::unique_ptr<QFile> file;
};
