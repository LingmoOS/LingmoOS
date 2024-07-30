/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "openvpnauth.h"
#include "passwordfield.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QString>

#include <KLocalizedString>

#include "nm-openvpn-service.h"
#include "plasma_nm_openvpn.h"

class OpenVpnAuthWidgetPrivate
{
public:
    NetworkManager::VpnSetting::Ptr setting;
    QFormLayout *layout;
};

OpenVpnAuthWidget::OpenVpnAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new OpenVpnAuthWidgetPrivate)
{
    Q_D(OpenVpnAuthWidget);
    d->setting = setting;
    d->layout = new QFormLayout(this);
    setLayout(d->layout);

    readSecrets();

    KAcceleratorManager::manage(this);
}

OpenVpnAuthWidget::~OpenVpnAuthWidget()
{
    delete d_ptr;
}

void OpenVpnAuthWidget::readSecrets()
{
    Q_D(OpenVpnAuthWidget);

    const NMStringMap dataMap = d->setting->data();
    const NMStringMap secrets = d->setting->secrets();
    const QString cType = dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE];
    NetworkManager::Setting::SecretFlags certType = (NetworkManager::Setting::SecretFlags)dataMap.value(NM_OPENVPN_KEY_CERTPASS "-flags").toInt();
    NetworkManager::Setting::SecretFlags passType = (NetworkManager::Setting::SecretFlags)dataMap.value(NM_OPENVPN_KEY_PASSWORD "-flags").toInt();
    NetworkManager::Setting::SecretFlags proxyType = (NetworkManager::Setting::SecretFlags)dataMap.value(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD "-flags").toInt();

    // If hints are given, then always ask for what the hints require
    if (!m_hints.isEmpty()) {
        const QString vpnMessage = QStringLiteral("x-vpn-message:");
        const QString tagDynChallNoEcho = QStringLiteral("x-dynamic-challenge:");
        const QString tagDynChallEcho = QStringLiteral("x-dynamic-challenge-echo:");
        std::optional<bool> passwordModeEnabled = std::nullopt;
        QString passwordType;
        QString prompt;

        for (const QString &hint : std::as_const(m_hints)) {
            if (hint.startsWith(vpnMessage)) {
                prompt = hint.sliced(vpnMessage.length());
            } else {
                if (hint.startsWith(tagDynChallNoEcho)) {
                    passwordModeEnabled = true;
                } else if (hint.startsWith(tagDynChallEcho)) {
                    passwordModeEnabled = false;
                }
                passwordType = hint;
            }
        }

        if (prompt.isEmpty()) {
            if (passwordType == QLatin1String(NM_OPENVPN_KEY_CERTPASS)) {
                prompt = i18n("Key Password:");
            } else if (passwordType == QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD)) {
                prompt = i18n("Proxy Password:");
            } else {
                prompt = i18n("Password:");
            }
        } else if (prompt.endsWith(QLatin1Char('.'))) {
            prompt = prompt.replace(prompt.length() - 1, 1, QLatin1Char(':'));
        } else if (!prompt.endsWith(QLatin1Char(':'))) {
            prompt += QLatin1Char(':');
        }

        if (!passwordModeEnabled.has_value()) {
            QStringList possibleTokens = {i18n("OTP"), i18n("authenticator"), i18n("code"), i18n("token"), i18n("one-time password")};

            for (const QString &possibleToken : possibleTokens) {
                if (prompt.toLower().contains(possibleToken.toLower())) {
                    passwordModeEnabled = false;
                    break;
                }
            }
        }

        addPasswordField(prompt, QString(), passwordType, passwordModeEnabled.value_or(true));
    } else {
        if (cType == QLatin1String(NM_OPENVPN_CONTYPE_TLS) || cType == QLatin1String(NM_OPENVPN_CONTYPE_PASSWORD_TLS)) {
            // Normal user password
            if (cType == QLatin1String(NM_OPENVPN_CONTYPE_PASSWORD_TLS) && !passType.testFlag(NetworkManager::Setting::NotRequired)) {
                addPasswordField(i18n("Password:"), secrets.value(QStringLiteral(NM_OPENVPN_KEY_PASSWORD)), QLatin1String(NM_OPENVPN_KEY_PASSWORD));
            }
            // Encrypted private key password
            if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_KEY)) && !certType.testFlag(NetworkManager::Setting::NotRequired)) {
                addPasswordField(i18n("Key Password:"), secrets.value(QStringLiteral(NM_OPENVPN_KEY_CERTPASS)), QLatin1String(NM_OPENVPN_KEY_CERTPASS));
            }
        } else if (cType == QLatin1String(NM_OPENVPN_CONTYPE_PASSWORD)) {
            addPasswordField(i18n("Password:"), secrets.value(QStringLiteral(NM_OPENVPN_KEY_PASSWORD)), QLatin1String(NM_OPENVPN_KEY_PASSWORD));
        }

        if (dataMap.contains(NM_OPENVPN_KEY_PROXY_SERVER) && !proxyType.testFlag(NetworkManager::Setting::NotRequired)) {
            addPasswordField(i18n("Proxy Password:"),
                             secrets.value(QStringLiteral(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD)),
                             QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD));
        }
    }

    for (int i = 0; i < d->layout->rowCount(); i++) {
        auto le = qobject_cast<PasswordField *>(d->layout->itemAt(i, QFormLayout::FieldRole)->widget());
        if (le && le->text().isEmpty()) {
            le->setFocus(Qt::OtherFocusReason);
            break;
        }
    }
}

QVariantMap OpenVpnAuthWidget::setting() const
{
    Q_D(const OpenVpnAuthWidget);

    NMStringMap secrets;
    QVariantMap secretData;
    for (int i = 0; i < d->layout->rowCount(); i++) {
        auto le = qobject_cast<PasswordField *>(d->layout->itemAt(i, QFormLayout::FieldRole)->widget());
        if (le && !le->text().isEmpty()) {
            const QString key = le->property("nm_secrets_key").toString();
            secrets.insert(key, le->text());
        }
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));
    return secretData;
}

void OpenVpnAuthWidget::addPasswordField(const QString &labelText, const QString &password, const QString &secretKey, bool passwordMode)
{
    Q_D(const OpenVpnAuthWidget);

    auto label = new QLabel(this);
    label->setText(labelText);
    auto lineEdit = new PasswordField(this);
    lineEdit->setPasswordModeEnabled(passwordMode);
    lineEdit->setProperty("nm_secrets_key", secretKey);
    lineEdit->setText(password);
    d->layout->addRow(label, lineEdit);
}

#include "moc_openvpnauth.cpp"
