// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PERMISSION_CLIENT_H_
#define PERMISSION_CLIENT_H_

#include <QObject>

typedef QVector<int> IntArray;

class PermissionClient : public QObject {
    Q_OBJECT
public:
    explicit PermissionClient(QObject* parent = nullptr);
    ~PermissionClient();

public Q_SLOTS: // METHODS
    QString ShowDisablePermissionDialog(const QString &title, const QString &description, const QStringList& options);
    QString Request(const QString &title, const QString &description, const QString &prefer, const QStringList& options);
};

#endif  // PERMISSION_CLIENT_H_
