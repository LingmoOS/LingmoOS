// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMPRESSORTEST_H
#define COMPRESSORTEST_H

#include <QWidget>
#include <QFileInfo>
#include "darchivemanager.h"
DCOMPRESSOR_USE_NAMESPACE

class CompressorTest : public QWidget
{
    Q_OBJECT
public:
    explicit CompressorTest(QWidget *parent = nullptr);

private:
    DArchiveManager::MgrFileEntry fileInfo2Entry(const QFileInfo &fileInfo);
};

#endif   // COMPRESSORTEST_H
