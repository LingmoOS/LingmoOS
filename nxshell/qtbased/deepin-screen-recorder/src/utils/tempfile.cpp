// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tempfile.h"

#include <QTemporaryFile>
#include <QDebug>

TempFile *TempFile::instance()
{
    static TempFile *tempFile = new TempFile;
    return tempFile;
}

TempFile::TempFile(QObject *parent)
    : QObject(parent)
{
}

TempFile::~TempFile()
{
}

void TempFile::setFullScreenPixmap(const QPixmap &pixmap)
{
    m_fullscreenPixmap = pixmap;
}

void TempFile::setBlurPixmap(const QPixmap &pixmap, const int radius)
{
    if (m_blurPixmap.contains(radius))
        return;
    m_blurPixmap[radius] = pixmap;
}

void TempFile::setMosaicPixmap(const QPixmap &pixmap, const int radius)
{
    if(m_mosaicPixmap.contains(radius))
        return;
    m_mosaicPixmap[radius] = pixmap;
}

bool TempFile::isContainsPix(const QString effect, const int radius)
{
    if (effect == "blur") {
        return m_blurPixmap.contains(radius);
    } else {
        return m_mosaicPixmap.contains(radius);
    }
}
