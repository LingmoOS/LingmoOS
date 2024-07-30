/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

// lingmo
#include <Lingmo5Support/DataContainer>

#include <KService>
#include <KServiceGroup>

/**
 * App Source
 */
class AppSource : public Lingmo5Support::DataContainer
{
    Q_OBJECT

public:
    AppSource(const KServiceGroup::Ptr &startup, QObject *parent);
    AppSource(const KService::Ptr &app, QObject *parent);
    ~AppSource() override;

protected:
    Lingmo5Support::Service *createService();
    KService::Ptr getApp();
    bool isApp() const;

private Q_SLOTS:
    void updateGroup();
    void updateApp();

private:
    friend class AppsEngine;
    friend class AppJob;
    KServiceGroup::Ptr m_group;
    KService::Ptr m_app;
    bool m_isApp;
};
