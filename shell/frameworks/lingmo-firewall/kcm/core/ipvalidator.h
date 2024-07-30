// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Daniel Vrátil <dvratil@kde.org>

#include "kcm_firewall_core_export.h"

#include <QValidator>

class KCM_FIREWALL_CORE_EXPORT IPValidator : public QValidator
{
    Q_OBJECT

    Q_PROPERTY(IPVersion ipVersion READ ipVersion WRITE setIPVersion NOTIFY ipVersionChanged)
public:
    enum class IPVersion { IPv4, IPv6 };
    Q_ENUM(IPVersion)

    explicit IPValidator(QObject *parent = nullptr);

    IPVersion ipVersion() const;
    void setIPVersion(IPVersion version);

    QValidator::State validate(QString &input, int &pos) const override;

Q_SIGNALS:
    void ipVersionChanged(IPVersion version);

private:
    IPVersion mIPVersion = IPVersion::IPv4;
};