// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAYICON_ICONPROVIDER_H
#define TRAYICON_ICONPROVIDER_H

#include <QIcon>
#include <QColor>
#include <QQuickImageProvider>

class IconProvider : public QQuickImageProvider
{
public:
    explicit IconProvider();
    ~IconProvider() override;

    QPixmap requestPixmap(const QString &icon, QSize *size, const QSize &requestedSize) override;

private:
    QColor getPenColor();

private:
    QColor penColor_;
};

#endif // !TRAYICON_ICONPROVIDER_H
