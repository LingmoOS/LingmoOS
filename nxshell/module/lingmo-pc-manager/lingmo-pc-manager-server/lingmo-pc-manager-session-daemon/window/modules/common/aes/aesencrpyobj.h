// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AESENCRPYOBJ_H
#define AESENCRPYOBJ_H

#include "aes.hpp"

#include <QObject>

class AesEncrpyObj : public QObject
{
    Q_OBJECT
public:
    explicit AesEncrpyObj(QObject *parent = nullptr);

public:
    void initKey();
    QByteArray encryptData(const QByteArray &strData);
    QByteArray decryptData(const QByteArray &byData);

private:
    struct AES_ctx m_ctx;
};

#endif // AESENCRPYOBJ_H
