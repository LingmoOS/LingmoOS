// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontloadthread.h"
#include <QFile>

DFontLoadThread::DFontLoadThread(QObject *parent)
    : QThread(parent)
{

}

DFontLoadThread::~DFontLoadThread()
{
}

void DFontLoadThread::open(const QString &filepath)
{
    m_filePath = filepath;
}

void DFontLoadThread::run()
{
    QFile file(m_filePath);

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray fileContent = file.readAll();

        emit loadFinished(fileContent);
    } else {
        emit loadFinished(QByteArray());
    }

    file.close();
}
