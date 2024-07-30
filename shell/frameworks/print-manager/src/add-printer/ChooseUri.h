/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHOOSE_URI_H
#define CHOOSE_URI_H

#include "GenericPage.h"

#include <KCupsPrinter.h>

#include <QUrl>

namespace Ui
{
class ChooseUri;
}
class KCupsRequest;
class ChooseUri : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseUri(QWidget *parent = nullptr);
    ~ChooseUri() override;

    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool isValid() const override;
    bool canProceed() const override;

public slots:
    void load();

signals:
    void errorMessage(const QString &message);
    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const KCupsPrinters &grouped_printers);

private slots:
    void checkSelected();
    void textChanged(const QString &text);
    void findPrinters();
    void getPrintersFinished(KCupsRequest *request);

private:
    QUrl parsedURL(const QString &text) const;

    Ui::ChooseUri *const ui;
};

#endif
