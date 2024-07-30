// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

class DirModelUtils : public QObject
{
    Q_OBJECT

public:
    explicit DirModelUtils(QObject *parent = nullptr);

    Q_INVOKABLE QStringList getUrlParts(const QUrl &url) const;
    Q_INVOKABLE QUrl partialUrlForIndex(QUrl url, int index) const;
    Q_INVOKABLE QUrl directoryOfUrl(const QString &path) const;
    Q_INVOKABLE QString fileNameOfUrl(const QString &path) const;

    Q_INVOKABLE void mkdir(const QUrl path) const;

Q_SIGNALS:
    void homePathChanged();
};
