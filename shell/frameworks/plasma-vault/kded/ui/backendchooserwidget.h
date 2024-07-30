/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_BACKEND_CHOOSER_WIDGET_H
#define PLASMAVAULT_KDED_UI_BACKEND_CHOOSER_WIDGET_H

#include "dialogdsl.h"

class BackendChooserWidget : public DialogDsl::DialogModule
{
    Q_OBJECT

public:
    BackendChooserWidget();
    ~BackendChooserWidget() override;

    void addItem(const QByteArray &id, const QString &title, int priority);

    PlasmaVault::Vault::Payload fields() const override;

    void checkBackendAvailable();

private Q_SLOTS:
    void checkCurrentBackend();
    void showBackendSelector();

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // include guard
