/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <Lingmo5Support/DataEngine>

class Klipper;
class HistoryModel;

class ClipboardEngine : public Lingmo5Support::DataEngine
{
    Q_OBJECT
public:
    ClipboardEngine(QObject *parent);
    ~ClipboardEngine() override;

    Lingmo5Support::Service *serviceForSource(const QString &source) override;

private:
    Klipper *m_klipper;
    std::shared_ptr<HistoryModel> m_model;
};
