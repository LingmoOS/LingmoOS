// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPRESSORAPPLICATION_H
#define COMPRESSORAPPLICATION_H

#include <DApplication>

DWIDGET_USE_NAMESPACE

// 应用程序
class CompressorApplication: public DApplication
{
    Q_OBJECT
public:
    explicit CompressorApplication(int &argc, char **argv);
    ~CompressorApplication() override;
    bool notify(QObject *watched, QEvent *event) override;

protected:
    void handleQuitAction() override;
};

#endif // COMPRESSORAPPLICATION_H
