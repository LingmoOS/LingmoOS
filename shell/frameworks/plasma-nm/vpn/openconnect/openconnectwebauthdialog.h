/*
    SPDX-FileCopyrightText: 2024 Joel Holdsworth <joel@airwebreathe.org.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef OPENCONNECTWEBAUTHDIALOG_H
#define OPENCONNECTWEBAUTHDIALOG_H

#include <QDialog>
#include <QWebEngineWebAuthUxRequest>

#include "ui_openconnectwebauthdialog.h"

class OpenconnectWebAuthDialogPrivate;

class OpenconnectWebAuthDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(OpenconnectWebAuthDialog)
public:
    OpenconnectWebAuthDialog(QWebEngineWebAuthUxRequest *request, QWidget *parent = nullptr);
    ~OpenconnectWebAuthDialog();

    void updateDisplay();

private:
    OpenconnectWebAuthDialogPrivate *const d_ptr;

    void setupSelectAccountUI();
    void setupCollectPinUI();
    void setupFinishCollectTokenUI();
    void setupErrorUI();
    void onCancelRequest();
    void onRetry();
    void onAcceptRequest();
    void clearSelectAccountButtons();
};

#endif // OPENCONNECTWEBAUTHDIALOG_H
