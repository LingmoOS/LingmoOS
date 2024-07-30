/*
    SPDX-FileCopyrightText: 2011 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pindialog.h"

#include <QIcon>
#include <QIntValidator>
#include <QScreen>
#include <QStringBuilder>

#include <KLocalizedString>
#include <KWindowSystem>
#include <KX11Extras>

#include <ModemManagerQt/Manager>

PinDialog::PinDialog(ModemManager::Modem *modem, const Type type, QWidget *parent)
    : QDialog(parent)
    , m_type(type)
{
    if (modem) {
        m_udi = modem->uni();
        m_name = modem->device();
        for (const Solid::Device &d : Solid::Device::allDevices()) {
            if (d.udi().contains(m_name, Qt::CaseInsensitive)) {
                m_name = d.product();
                if (!m_name.startsWith(d.vendor())) {
                    m_name = d.vendor() % ' ' % m_name;
                }
                break;
            }
        }
    }

    ui = new Ui::PinWidget();
    ui->setupUi(this);
    ui->pin->setEchoMode(QLineEdit::Password);

    auto validator = new QIntValidator(this);
    validator->setRange(1000, 99999999);
    ui->pin->setValidator(validator);
    ui->pin2->setValidator(validator);

    auto validator2 = new QIntValidator(this);
    validator2->setRange(10000000, 99999999);
    ui->puk->setValidator(validator2);

    ui->errorMessage->setHidden(true);
    const QRect desktop = screen()->availableGeometry();
    // QRect desktop = QApplication::desktop()->screenGeometry(topLevelWidget());
    setMinimumWidth(360); // width of the PinePhone display

    pixmapLabel = new QLabel(this);
    pixmapLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->gridLayout->addWidget(pixmapLabel, 0, 0);
    pixmapLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-password")).pixmap(32));

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &PinDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &PinDialog::reject);

    if (isPukDialog()) {
        QString pukType;
        if (m_type == PinDialog::SimPuk) {
            pukType = i18n("SIM PUK");
        } else if (m_type == PinDialog::SimPuk2) {
            pukType = i18n("SIM PUK2");
        } else if (m_type == PinDialog::ModemServiceProviderPuk) {
            pukType = i18n("Service provider PUK");
        } else if (m_type == PinDialog::ModemNetworkPuk) {
            pukType = i18n("Network PUK");
        } else if (m_type == PinDialog::ModemCorporatePuk) {
            pukType = i18n("Corporate PUK");
        } else if (m_type == PinDialog::ModemPhFsimPuk) {
            pukType = i18n("PH-FSIM PUK");
        } else {
            pukType = i18n("Network Subset PUK");
        }

        setWindowTitle(i18n("%1 unlock required", pukType));
        ui->title->setText(i18n("%1 Unlock Required", pukType));
        ui->prompt->setText(i18n("The mobile broadband device '%1' requires a %2 code before it can be used.", m_name, pukType));
        ui->pukLabel->setText(i18n("%1 code:", pukType));
        ui->pinLabel->setText(i18n("New PIN code:"));
        ui->pin2Label->setText(i18n("Re-enter new PIN code:"));
        ui->chkShowPass->setText(i18n("Show PIN/PUK code"));

        ui->puk->setFocus();
        ui->pukLabel->show();
        ui->puk->show();
        ui->pin2Label->show();
        ui->pin2->show();
    } else if (isPinDialog()) {
        QString pinType;
        if (m_type == PinDialog::SimPin) {
            pinType = i18n("SIM PIN");
        } else if (m_type == PinDialog::SimPin2) {
            pinType = i18n("SIM PIN2");
        } else if (m_type == PinDialog::ModemServiceProviderPin) {
            pinType = i18n("Service provider PIN");
        } else if (m_type == PinDialog::ModemNetworkPin) {
            pinType = i18n("Network PIN");
        } else if (m_type == PinDialog::ModemPin) {
            pinType = i18n("PIN");
        } else if (m_type == PinDialog::ModemCorporatePin) {
            pinType = i18n("Corporate PIN");
        } else if (m_type == PinDialog::ModemPhFsimPin) {
            pinType = i18n("PH-FSIM PIN");
        } else {
            pinType = i18n("Network Subset PIN");
        }
        setWindowTitle(i18n("%1 unlock required", pinType));
        ui->title->setText(i18n("%1 Unlock Required", pinType));
        ui->prompt->setText(i18n("The mobile broadband device '%1' requires a %2 code before it can be used.", m_name, pinType));
        ui->pinLabel->setText(i18n("%1 code:", pinType));
        ui->chkShowPass->setText(i18n("Show PIN code"));

        ui->pin->setFocus();
        ui->pukLabel->hide();
        ui->puk->hide();
        ui->pin2Label->hide();
        ui->pin2->hide();
    }

    ui->puk->clear();
    ui->pin->clear();
    ui->pin2->clear();
    ui->puk->setCursorPosition(0);
    ui->pin->setCursorPosition(0);
    ui->pin2->setCursorPosition(0);

    if (KWindowSystem::isPlatformX11()) {
        KX11Extras::setState(winId(), NET::KeepAbove);
    }

    move((desktop.width() - width()) / 2, (desktop.height() - height()) / 2);
    connect(ui->chkShowPass, &QCheckBox::toggled, this, &PinDialog::chkShowPassToggled);
    connect(ModemManager::notifier(), &ModemManager::Notifier::modemRemoved, this, &PinDialog::modemRemoved);
}

PinDialog::~PinDialog()
{
    delete ui;
}

void PinDialog::chkShowPassToggled(bool on)
{
    ui->pin->setEchoMode(!on ? QLineEdit::Password : QLineEdit::Normal);
    ui->pin2->setEchoMode(!on ? QLineEdit::Password : QLineEdit::Normal);
    ui->puk->setEchoMode(!on ? QLineEdit::Password : QLineEdit::Normal);

    ui->puk->setCursorPosition(0);
    ui->pin->setCursorPosition(0);
    ui->pin2->setCursorPosition(0);

    if (isPukDialog()) {
        ui->puk->setFocus();
    } else {
        ui->pin->setFocus();
    }
}

void PinDialog::modemRemoved(const QString &udi)
{
    if (udi == m_udi) {
        reject();
    }
}

PinDialog::Type PinDialog::type() const
{
    return m_type;
}

QString PinDialog::pin() const
{
    return ui->pin->text();
}

QString PinDialog::pin2() const
{
    return ui->pin2->text();
}

QString PinDialog::puk() const
{
    return ui->puk->text();
}

void PinDialog::showErrorMessage(const PinDialog::ErrorCode error)
{
    QString msg;
    QFont bold = font();
    ui->pinLabel->setFont(bold);
    ui->pin2Label->setFont(bold);
    ui->pukLabel->setFont(bold);
    bold.setBold(true);
    switch (error) {
    case PinCodeTooShort:
        msg = i18n("PIN code too short. It should be at least 4 digits.");
        ui->pin->setFocus();
        ui->pinLabel->setFont(bold);
        break;
    case PinCodesDoNotMatch:
        msg = i18n("The two PIN codes do not match");
        ui->pin2->setFocus();
        ui->pin2Label->setFont(bold);
        break;
    case PukCodeTooShort:
        msg = i18n("PUK code too short. It should be 8 digits.");
        ui->puk->setFocus();
        ui->pukLabel->setFont(bold);
        break;
    default:
        msg = i18n("Unknown Error");
    }
    ui->errorMessage->setText(msg, KTitleWidget::ErrorMessage);
    adjustSize();
}

void PinDialog::accept()
{
    if (isPukDialog()) {
        if (pin() != pin2()) {
            showErrorMessage(PinCodesDoNotMatch);
            return;
        } else if (puk().length() < 8) {
            showErrorMessage(PukCodeTooShort);
            return;
        }
    }

    if (pin().length() < 4) {
        showErrorMessage(PinCodeTooShort);
        return;
    }

    QDialog::accept();
}

bool PinDialog::isPinDialog() const
{
    return (m_type == PinDialog::SimPin || //
            m_type == PinDialog::SimPin2 || //
            m_type == PinDialog::ModemServiceProviderPin || //
            m_type == PinDialog::ModemNetworkPin || //
            m_type == PinDialog::ModemPin || //
            m_type == PinDialog::ModemCorporatePin || //
            m_type == PinDialog::ModemPhFsimPin || //
            m_type == PinDialog::ModemNetworkSubsetPin);
}

bool PinDialog::isPukDialog() const
{
    return !isPinDialog();
}

#include "moc_pindialog.cpp"
