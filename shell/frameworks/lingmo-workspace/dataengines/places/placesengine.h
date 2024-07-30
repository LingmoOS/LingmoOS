/*
    SPDX-FileCopyrightText: 2008 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <Lingmo5Support/DataEngine>

#include <kfileplacesmodel.h>

class PlacesProxyModel;

class PlacesEngine : public Lingmo5Support::DataEngine
{
    Q_OBJECT

public:
    PlacesEngine(QObject *parent);
    ~PlacesEngine() override;

    Lingmo5Support::Service *serviceForSource(const QString &source) override;

private:
    KFilePlacesModel *m_placesModel = nullptr;
    PlacesProxyModel *m_proxyModel = nullptr;
};
