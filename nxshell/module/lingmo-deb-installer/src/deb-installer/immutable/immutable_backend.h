// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMMUTABLEBACKEND_H
#define IMMUTABLEBACKEND_H

#include <QObject>

namespace Immutable {

class ImmutableBackend : public QObject
{
    Q_OBJECT

public:
    static ImmutableBackend *instance();

    [[nodiscard]] bool immutableEnabled() const;

private:
    explicit ImmutableBackend(QObject *parent = nullptr);
    ~ImmutableBackend() override = default;

    void initBackend();

    bool m_immutableEnabled{false};

    Q_DISABLE_COPY(ImmutableBackend);
};

};  // namespace Immutable

using ImmBackend = Immutable::ImmutableBackend;

#endif  // IMMUTABLEBACKEND_H
