/*
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KURLHANDLER_P_H
#define KURLHANDLER_P_H

#include <kguiaddons_export.h>

#include <QObject>

class QUrl;

class KGUIADDONS_EXPORT KUrlHandler : public QObject
{
    Q_OBJECT

public:
    explicit KUrlHandler(QObject *parent = nullptr);

    QUrl concatDocsUrl(const QUrl &url) const;

public Q_SLOTS:
    void openHelp(const QUrl &url) const;
};

#endif // KURLHANDLER_P_H
