// SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QDBusPendingReply>
#include <QPointer>

#include <KConfigGroup>
#include <KIO/OpenWithHandlerInterface>

class KIOOpenWith : public KIO::OpenWithHandlerInterface
{
    Q_OBJECT
public:
    explicit KIOOpenWith(QWidget *parentWidget, QObject *parent = nullptr);
    void promptUserForApplication(KJob *job, const QList<QUrl> &urls, const QString &mimeType) override;

private Q_SLOTS:
    void onApplicationChosen(const QDBusPendingReply<uint, QVariantMap> &reply, KConfigGroup cg, const QString &mimeType, QWidget *widget);

private:
    [[nodiscard]] KService::Ptr makeService(const QVariantMap &resultMap, const QString &mimeType, QWidget *widget);
    QPointer<QWidget> m_parentWidget;
};
