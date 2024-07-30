/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "requestpin.h"
#include "bluedevil_kded.h"
#include "ui_requestpin.h"

#include <QDialog>
#include <QIcon>
#include <QPushButton>
#include <QRegularExpressionValidator>

#include <KLocalizedString>
#include <KNotification>
#include <KX11Extras>

RequestPin::RequestPin(BluezQt::DevicePtr device, bool numeric, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_numeric(numeric)
{
    m_notification = new KNotification(QStringLiteral("RequestPin"), KNotification::Persistent, this);

    m_notification->setComponentName(QStringLiteral("bluedevil"));
    m_notification->setTitle(QStringLiteral("%1 (%2)").arg(m_device->name().toHtmlEscaped(), m_device->address().toHtmlEscaped()));
    m_notification->setText(
        i18nc("Shown in a notification to announce that a PIN is needed to accomplish a pair action,"
              "%1 is the name of the bluetooth device",
              "PIN needed to pair with %1",
              m_device->name().toHtmlEscaped()));

    auto action = m_notification->addAction(i18nc("Notification button which once clicked, a dialog to introduce the PIN will be shown", "Introduce PIN"));

    connect(action, &KNotificationAction::activated, this, &RequestPin::introducePin);
    connect(m_notification, &KNotification::closed, this, &RequestPin::quit);
    connect(m_notification, &KNotification::ignored, this, &RequestPin::quit);
    connect(parent, SIGNAL(agentCanceled()), this, SLOT(quit()));

    m_notification->sendEvent();
}

void RequestPin::introducePin()
{
    m_notification->disconnect();
    m_notification->close();
    m_notification->deleteLater();

    QDialog *dialog = new QDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));

    dialog->setWindowTitle(i18nc("Shown in the caption of a dialog where the user introduce the PIN", "Introduce PIN"));

    m_dialogWidget = new Ui::DialogWidget;
    m_dialogWidget->setupUi(dialog);
    m_dialogWidget->descLabel->setText(
        i18nc("Shown in a dialog which asks to introduce a PIN that will be used to pair a Bluetooth device,"
              "%1 is the name of the Bluetooth device",
              "In order to pair this computer with %1, you have to enter a PIN. Please do it below.",
              m_device->name()));

    m_dialogWidget->pixmap->setPixmap(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")).pixmap(64));
    m_dialogWidget->pin->setFocus(Qt::ActiveWindowFocusReason);

    if (m_numeric) {
        QRegularExpression rx(QStringLiteral("[0-9]{1,6}"));
        m_dialogWidget->pin->setValidator(new QRegularExpressionValidator(rx, this));
    } else {
        QRegularExpression rx(QStringLiteral("[A-Za-z0-9]{1,16}"));
        m_dialogWidget->pin->setValidator(new QRegularExpressionValidator(rx, this));
    }

    m_dialogWidget->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    dialog->setFixedSize(dialog->sizeHint());

    connect(dialog, &QDialog::finished, this, &RequestPin::dialogFinished);
    connect(m_dialogWidget->pin, &QLineEdit::textChanged, this, &RequestPin::checkPin);
    connect(m_dialogWidget->buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(m_dialogWidget->buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    dialog->setWindowFlags(Qt::WindowStaysOnTopHint);

    dialog->show();

    KX11Extras::forceActiveWindow(dialog->winId());
}

void RequestPin::checkPin(const QString &pin)
{
    m_dialogWidget->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!pin.isEmpty());
}

void RequestPin::dialogFinished(int result)
{
    deleteLater();

    if (!result) {
        qCDebug(BLUEDEVIL_KDED_LOG) << "PIN dialog rejected:" << m_device->name() << m_device->address();
        Q_EMIT done(QString());
        return;
    }

    qCDebug(BLUEDEVIL_KDED_LOG) << "PIN dialog accepted:" << m_device->name() << m_device->address();
    Q_EMIT done(m_dialogWidget->pin->text().toLatin1().constData());
}

void RequestPin::quit()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "Rejected to introduce PIN:" << m_device->name() << m_device->address();

    deleteLater();
    Q_EMIT done(QString());
}

#include "moc_requestpin.cpp"
