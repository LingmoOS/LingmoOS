/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAGE_DESTINATIONS_H
#define PAGE_DESTINATIONS_H

#include "GenericPage.h"

#include <KCupsPrinter.h>

#include <QLabel>
#include <QUrl>

namespace Ui
{
class PageDestinations;
}

class DevicesModel;
class ChooseLpd;
class ChooseSamba;
class ChooseSerial;
class ChooseSocket;
class ChooseUri;
class PageDestinations : public GenericPage
{
    Q_OBJECT
public:
    explicit PageDestinations(const QVariantMap &args = QVariantMap(), QWidget *parent = nullptr);
    ~PageDestinations() override;

    bool canProceed() const override;
    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool isValid() const override;

private slots:
    void deviceChanged();
    void deviceUriChanged();
    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const KCupsPrinters &grouped_printers);

private:
    QVariantMap selectedItemValues() const;
    void setCurrentPage(QWidget *widget, const QVariantMap &args);

    Ui::PageDestinations *const ui;
    DevicesModel *m_model = nullptr;

    QString m_currentUri;
    ChooseLpd *m_chooseLpd = nullptr;
    ChooseSamba *m_chooseSamba = nullptr;
    ChooseSerial *m_chooseSerial = nullptr;
    ChooseSocket *m_chooseSocket = nullptr;
    ChooseUri *m_chooseUri = nullptr;
    QLabel *m_chooseLabel = nullptr;
};

#endif
