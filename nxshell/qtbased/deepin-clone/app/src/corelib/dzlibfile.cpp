// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "dzlibfile.h"

#include <QDebug>

DZlibFile::DZlibFile(const QString &name)
    : m_file(new QFile(this))
{
    setFileName(name);
    setDevice(m_file);
}

DZlibFile::DZlibFile(QObject *parent)
    : DZlibIODevice(parent)
    , m_file(new QFile(this))
{
    setDevice(m_file);
}

DZlibFile::DZlibFile(const QString &name, QObject *parent)
    : DZlibIODevice(parent)
    , m_file(new QFile(this))
{
    setFileName(name);
    setDevice(m_file);
}

void DZlibFile::setFileName(const QString &name)
{
    if (isOpen()) {
        qWarning("DZlibFile::setFileName: File (%s) is already opened",
                 qPrintable(m_file->fileName()));
        close();
    }

    m_file->setFileName(name);
    setDevice(m_file);
}

QString DZlibFile::fileName() const
{
    return m_file->fileName();
}
