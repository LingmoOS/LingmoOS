// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTLOADTHREAD_H
#define DFONTLOADTHREAD_H

#include <QThread>

class DFontLoadThread : public QThread
{
    Q_OBJECT

public:
    explicit DFontLoadThread(QObject *parent = nullptr);
    ~DFontLoadThread();

    void run();
    void open(const QString &filepath);

signals:
    void loadFinished(const QByteArray &data);

private:
    QString m_filePath;
};

#endif
