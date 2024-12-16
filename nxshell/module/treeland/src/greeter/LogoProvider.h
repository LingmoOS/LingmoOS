// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef LOGOPROVIDER_H
#define LOGOPROVIDER_H

#include <QQmlEngine>

class logoProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit logoProvider(QObject *parent = nullptr)
        : QObject(parent){};

    Q_PROPERTY(QString logo READ logo CONSTANT)
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
    [[nodiscard]] static QString logo() noexcept;
    [[nodiscard]] QString version() const noexcept;
    Q_INVOKABLE void updateLocale(const QLocale &newLocale) noexcept;

Q_SIGNALS:
    void versionChanged();

private:
    QLocale locale{ QLocale::system() };
};

#endif // LOGOPROVIDER_H
