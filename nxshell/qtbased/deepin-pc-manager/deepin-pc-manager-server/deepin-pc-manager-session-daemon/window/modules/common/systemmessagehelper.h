// Copyright (C) 2019 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMMESSAGEHELPER_H
#define SYSTEMMESSAGEHELPER_H

#include <QObject>

class SystemMessageHelper : public QObject
{
    Q_OBJECT
public:
    explicit SystemMessageHelper(QObject *parent = nullptr);

    static void createMessageForAVEngineExpiredInDays(const QString &, int days);
};

#endif // SYSTEMMESSAGEHELPER_H
