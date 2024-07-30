/*
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QQuickItem>
#include <QWindow>

#include <Lingmo/Applet>

class QString;
class QRect;

class NotificationApplet : public Lingmo::Applet
{
    Q_OBJECT

    Q_PROPERTY(QWindow *focussedLingmoDialog READ focussedLingmoDialog NOTIFY focussedLingmoDialogChanged)
    Q_PROPERTY(QQuickItem *systemTrayRepresentation READ systemTrayRepresentation CONSTANT)

public:
    explicit NotificationApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~NotificationApplet() override;

    void init() override;
    void configChanged() override;

    QWindow *focussedLingmoDialog() const;
    QQuickItem *systemTrayRepresentation() const;

    Q_INVOKABLE bool isPrimaryScreen(const QRect &rect) const;

    Q_INVOKABLE void forceActivateWindow(QWindow *window);

Q_SIGNALS:
    void focussedLingmoDialogChanged();
};
