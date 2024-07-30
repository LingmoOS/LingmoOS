/*
    SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <LingmoQuick/Dialog>

class SubDialog : public LingmoQuick::Dialog
{
    Q_OBJECT

public:
    explicit SubDialog(QQuickItem *parent = nullptr);
    ~SubDialog() override;

    Q_INVOKABLE QRect availableScreenRectForItem(QQuickItem *item) const;

    QPoint popupPosition(QQuickItem *item, const QSize &size) override;
};
