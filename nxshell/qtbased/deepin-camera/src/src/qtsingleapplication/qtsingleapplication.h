// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#define protected public
#include <DApplication>
#undef protected

DWIDGET_USE_NAMESPACE

class QtSingleApplication : public QObject
{
    Q_OBJECT

public:
    QtSingleApplication(int &argc, char **argv);

    ~QtSingleApplication();

    DApplication *dApplication();

private:
    DApplication *_app;
};

#endif // QTSINGLEAPPLICATION_H
