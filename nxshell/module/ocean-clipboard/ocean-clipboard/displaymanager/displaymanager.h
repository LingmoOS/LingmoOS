// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <QObject>

#include "constants.h"

class QScreen;
class DisplayManager: public QObject
{
    Q_OBJECT
public:
    static DisplayManager *instance();

private:
    explicit DisplayManager(QObject *parent = Q_NULLPTR);

private Q_SLOTS:
    void screenCountChanged();

Q_SIGNALS:
    void screenInfoChanged();

private:
    QList<QScreen *> m_screens;
};

#endif // DISPLAYMANAGER_H
