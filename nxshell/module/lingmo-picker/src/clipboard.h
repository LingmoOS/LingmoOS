// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QColor>

class Clipboard : public QObject
{
    Q_OBJECT

public:
    explicit Clipboard(QObject *parent = nullptr);
    ~Clipboard();

public slots:
    void copyToClipboard(QColor color, QString colorString);
};

#endif
