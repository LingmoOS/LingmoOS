// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MIMEMANAGERSERVICE_H
#define MIMEMANAGERSERVICE_H

#include <QObject>
#include <QDBusContext>
#include <QDBusObjectPath>
#include "global.h"
#include "applicationmimeinfo.h"

class ApplicationManager1Service;

class MimeManager1Service : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    explicit MimeManager1Service(ApplicationManager1Service *parent);
    ~MimeManager1Service() override;

    void appendMimeInfo(MimeInfo &&info);
    [[nodiscard]] const auto &infos() const noexcept { return m_infos; }
    [[nodiscard]] auto &infos() noexcept { return m_infos; }
    void reset() noexcept;

public Q_SLOTS:
    [[nodiscard]] ObjectMap listApplications(const QString &mimeType) const noexcept;
    [[nodiscard]] QString queryDefaultApplication(const QString &content, QDBusObjectPath &application) const noexcept;
    void setDefaultApplication(const QStringMap &defaultApps) noexcept;
    void unsetDefaultApplication(const QStringList &mimeTypes) noexcept;

private:
    QMimeDatabase m_database;
    std::vector<MimeInfo> m_infos;
};

#endif
