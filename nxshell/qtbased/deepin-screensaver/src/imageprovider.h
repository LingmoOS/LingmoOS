// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>

class ImageProvider : public QQuickImageProvider
{
public:
    explicit ImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize);
};

#endif // IMAGEPROVIDER_H
