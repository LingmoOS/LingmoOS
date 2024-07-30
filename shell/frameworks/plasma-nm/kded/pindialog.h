/*
    SPDX-FileCopyrightText: 2011 Lamarque Souza <lamarque@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_PIN_DIALOG_H
#define PLASMA_NM_PIN_DIALOG_H

#include <QWidget>

#include <QDialog>
#include <QPushButton>
#include <Solid/Device>

#include "ui_pinwidget.h"
#include <ModemManagerQt/Modem>

class PinWidget;

class PinDialog : public QDialog
{
    Q_OBJECT
public:
    enum Type {
        SimPin,
        SimPin2,
        SimPuk,
        SimPuk2,
        ModemServiceProviderPin,
        ModemServiceProviderPuk,
        ModemNetworkPin,
        ModemNetworkPuk,
        ModemPin,
        ModemCorporatePin,
        ModemCorporatePuk,
        ModemPhFsimPin,
        ModemPhFsimPuk,
        ModemNetworkSubsetPin,
        ModemNetworkSubsetPuk,
    };
    enum ErrorCode { PinCodeTooShort, PinCodesDoNotMatch, PukCodeTooShort };

    explicit PinDialog(ModemManager::Modem *modem, const Type type = SimPin, QWidget *parent = nullptr);
    ~PinDialog() override;

    Type type() const;
    QString pin() const;
    QString pin2() const;
    QString puk() const;

public Q_SLOTS:
    void accept() override;

private Q_SLOTS:
    void chkShowPassToggled(bool on);
    void modemRemoved(const QString &udi);

private:
    void showErrorMessage(const PinDialog::ErrorCode);
    bool isPukDialog() const;
    bool isPinDialog() const;
    Ui::PinWidget *ui = nullptr;
    QLabel *pixmapLabel = nullptr;
    QString m_name;
    const Type m_type;
    QString m_udi;
};

#endif // PLASMA_NM_PIN_DIALOG_H
