// Copyright (C) 2024 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IMAGEHELPER_H
#define IMAGEHELPER_H

#include <QImage>
#include <QObject>
#include <QQmlEngine>

class ImageHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool debug MEMBER debug CONSTANT)
public:
    explicit ImageHelper(QObject *parent = nullptr);

    Q_INVOKABLE bool isDarkType(const QImage &img);

#ifdef QT_DEBUG
    bool debug = true;
#else
    bool debug = false;
#endif
};

#endif // IMAGEHELPER_H
