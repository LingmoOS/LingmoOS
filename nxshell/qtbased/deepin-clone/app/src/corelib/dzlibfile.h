// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DZLIBFILE_H
#define DZLIBFILE_H

#include "dzlibiodevice.h"

#include <QFile>

class DZlibFile : public DZlibIODevice
{
    Q_OBJECT
public:
    DZlibFile(const QString &name);
    explicit DZlibFile(QObject *parent = 0);
    DZlibFile(const QString &name, QObject *parent);

    void setFileName(const QString &name);
    QString fileName() const;

private:
    QFile *m_file;
};

#endif // DZLIBFILE_H
