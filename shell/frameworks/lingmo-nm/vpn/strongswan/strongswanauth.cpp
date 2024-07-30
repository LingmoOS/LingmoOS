/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "strongswanauth.h"
#include "nm-strongswan-service.h"
#include "ui_strongswanauth.h"

#include <KMessageBox>
#include <QDialog>
#include <QProcessEnvironment>
#include <QString>

#include <KLocalizedString>

class StrongswanAuthWidgetPrivate
{
public:
    Ui_StrongswanAuth ui;
    bool acceptOnShow;
    NetworkManager::VpnSetting::Ptr setting;
};

StrongswanAuthWidget::StrongswanAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new StrongswanAuthWidgetPrivate)
{
    Q_D(StrongswanAuthWidget);
    d->setting = setting;
    d->ui.setupUi(this);
    d->acceptOnShow = false;

    readSecrets();

    KAcceleratorManager::manage(this);
}

StrongswanAuthWidget::~StrongswanAuthWidget()
{
    delete d_ptr;
}

void StrongswanAuthWidget::readSecrets()
{
    Q_D(StrongswanAuthWidget);
    const NMStringMap dataMap = d->setting->data();

    const QString method = dataMap[NM_STRONGSWAN_METHOD];
    if (method == QLatin1String(NM_STRONGSWAN_AUTH_AGENT) || dataMap[NM_STRONGSWAN_SECRET_TYPE] == QLatin1String(NM_STRONGSWAN_PW_TYPE_UNUSED)) {
        if (isVisible()) {
            acceptDialog();
        } else {
            d->acceptOnShow = true;
        }
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_KEY)) {
        d->ui.passwordLabel->setText(i18nc("@label:textbox password label for private key password", "Private Key Password:"));
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_SMARTCARD)) {
        d->ui.passwordLabel->setText(i18nc("@label:textbox password label for smartcard pin", "PIN:"));
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_EAP)) {
        d->ui.passwordLabel->setText(i18nc("@label:textbox password label for EAP password", "Password:"));
    }
}

void StrongswanAuthWidget::setVisible(bool visible)
{
    Q_D(StrongswanAuthWidget);

    SettingWidget::setVisible(visible);

    if (visible) {
        if (d->acceptOnShow) {
            acceptDialog();
        } else {
            SettingWidget::setVisible(visible);
        }
    } else {
        SettingWidget::setVisible(visible);
    }
}

void StrongswanAuthWidget::acceptDialog()
{
    auto dialog = qobject_cast<QDialog *>(parentWidget());
    if (dialog) {
        dialog->accept();
    }
}

QVariantMap StrongswanAuthWidget::setting() const
{
    Q_D(const StrongswanAuthWidget);

    NMStringMap secrets;
    QVariantMap secretData;

    if (d->setting->data()[NM_STRONGSWAN_METHOD] == QLatin1String(NM_STRONGSWAN_AUTH_AGENT)) {
        const QString agent = QProcessEnvironment::systemEnvironment().value(QLatin1String("SSH_AUTH_SOCK"));
        if (!agent.isEmpty()) {
            secrets.insert(NM_STRONGSWAN_AUTH_AGENT, agent);
        } else {
            KMessageBox::error(nullptr,
                               i18nc("@label:textbox error message while saving configuration",
                                     "Configuration uses ssh-agent for authentication, but no ssh-agent found running."));
        }
    } else {
        secrets.insert(NM_STRONGSWAN_SECRET, d->ui.password->text());
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));
    return secretData;
}

#include "moc_strongswanauth.cpp"
