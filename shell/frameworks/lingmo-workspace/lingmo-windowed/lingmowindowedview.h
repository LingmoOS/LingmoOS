/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "lingmowindowedcorona.h"
#include <LingmoQuick/ConfigView>
#include <QPointer>
#include <QQuickView>

class KStatusNotifierItem;

class LingmoWindowedView : public QQuickView
{
    Q_OBJECT

public:
    explicit LingmoWindowedView(QWindow *parent = nullptr);
    ~LingmoWindowedView() override;

    void setApplet(Lingmo::Applet *applet);
    void setHasStatusNotifier(bool stay);
    void showConfigurationInterface();

protected:
    void resizeEvent(QResizeEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void moveEvent(QMoveEvent *ev) override;
    void hideEvent(QHideEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;

protected Q_SLOTS:
    void showConfigurationInterface(Lingmo::Applet *applet);
    void minimumWidthChanged();
    void minimumHeightChanged();
    void maximumWidthChanged();
    void maximumHeightChanged();

private:
    void updateSniIcon();
    void updateSniTitle();
    void updateSniStatus();

    Lingmo::Applet *m_applet;
    QPointer<QObject> m_layout;
    QPointer<LingmoQuick::ConfigView> m_configView;
    QPointer<QQuickItem> m_rootObject;
    QPointer<QQuickItem> m_appletInterface;
    QPointer<KStatusNotifierItem> m_statusNotifier;
    bool m_withStatusNotifier;
};
