/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OPENCONNECTAUTH_H
#define OPENCONNECTAUTH_H

#include "settingwidget.h"

#include <NetworkManagerQt/VpnSetting>

#include <QNetworkCookie>
#include <QSemaphore>
#include <QString>
#include <QUrl>
#include <QWebEngineLoadingInfo>
#include <QWebEngineWebAuthUxRequest>

class QLayout;
struct openconnect_info;
struct oc_auth_form;

class OpenconnectAuthWidgetPrivate;

class OpenconnectAuthWidget : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(OpenconnectAuthWidget)
public:
    explicit OpenconnectAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr);
    ~OpenconnectAuthWidget() override;
    virtual void readSecrets();
    void readConfig();
    QVariantMap setting() const override;

private:
    OpenconnectAuthWidgetPrivate *const d_ptr;
    void acceptDialog();
    void addFormInfo(const QString &, const QString &);
    void deleteAllFromLayout(QLayout *);

private Q_SLOTS:
    void writeNewConfig(const QString &);
    void validatePeerCert(const QString &, const QString &, const QString &, bool *);
    void processAuthForm(struct oc_auth_form *);
    void updateLog(const QString &, const int &);
    void logLevelChanged(int);
    void formLoginClicked();
    void formGroupChanged();
    void workerFinished(const int &);
    void viewServerLogToggled(bool);
    void connectHost();
    void initTokens();
    void handleWebEngineCookie(const QNetworkCookie &);
    void handleWebEngineLoad(const QWebEngineLoadingInfo &);
    void handleWebEngineUrl(const QUrl &url);
    void handleWebAuthUxRequested(QWebEngineWebAuthUxRequest *request);
    void handleWebAuthUxStateChanged(QWebEngineWebAuthUxRequest::WebAuthUxState state);
    void openWebEngine(const char *, QSemaphore *);
};

#endif // OPENCONNECTAUTH_H
