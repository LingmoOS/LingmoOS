// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEMO_H
#define DEMO_H

#include "ddemotypes.h"
#include <QSize>
#include <QObject>
#include <DExpected>

DDEMO_BEGIN_NAMESPACE

class DemoPrivate;

using DCORE_NAMESPACE::DExpected;

class Demo : public QObject
{
    Q_OBJECT
public:
    explicit Demo(QObject *parent = nullptr);
    ~Demo() override;
    int add(const int a, const int b);

    // librsvg
    bool svg2png(const QString &svgfile, const QString &pngfile, QSize size = QSize(128, 128));
    bool docked();

signals:
    void DockedChanged(const bool value);

public slots:
    DExpected<UserPathList> listUsers();

private:
    QScopedPointer<DemoPrivate> d_ptr{nullptr};
    Q_DECLARE_PRIVATE(Demo)
};
DDEMO_END_NAMESPACE

#endif
