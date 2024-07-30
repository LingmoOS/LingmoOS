/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QRect>
#include <QString>

#include <KScreen/Output>

#include <memory>

#include "../common/osdaction.h"

class QQuickView;

class QTimer;

namespace KScreen
{
class Osd : public QObject
{
    Q_OBJECT

public:
    explicit Osd(const OutputPtr &output, QObject *parent = nullptr);
    ~Osd() override;

    void showActionSelector();
    void hideOsd();

Q_SIGNALS:
    void osdActionSelected(OsdAction::Action action);

private Q_SLOTS:
    void onOsdActionSelected(int action);
    void onOutputAvailabilityChanged();

private:
    KScreen::OutputPtr m_output;
    QQmlEngine m_engine;
    std::unique_ptr<QQuickView> m_osdActionSelector;
    QTimer *m_osdTimer = nullptr;
    int m_timeout = 0;
};

} // ns
