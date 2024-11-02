/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SECURITYWIDGET_H
#define SECURITYWIDGET_H

#include <QWidget>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include "coninfo.h"
#include "kylable.h"
#include "kwidget.h"
#include "kpasswordedit.h"

using namespace kdk;

class SecurityPage : public QFrame
{
    Q_OBJECT
public:
    SecurityPage(bool isNetDetailPage, QWidget *parent = nullptr);

    void setSecurity(KySecuType index);
    void setPsk(const QString &psk);
    void setTlsInfo(KyEapMethodTlsInfo &info);
    void setPeapInfo(KyEapMethodPeapInfo &info);
    void setTtlsInfo(KyEapMethodTtlsInfo &info);
    void setLeapInfo(KyEapMethodLeapInfo &info);
    void setPwdInfo(KyEapMethodPwdInfo &info);
    void setFastInfo(KyEapMethodFastInfo &info);
    void setSecurityVisible(const bool &visible);

    bool checkIsChanged(const ConInfo info);
    void updateSecurityChange(KyWirelessConnectSetting &setting);
    void updateTlsChange(KyEapMethodTlsInfo &info);
    void updatePeapChange(KyEapMethodPeapInfo &info);
    void updateTtlsChange(KyEapMethodTtlsInfo &info);
    void updateLeapChange(KyEapMethodLeapInfo &info);
    void updatePwdChange(KyEapMethodPwdInfo &info);
    void updateFastChange(KyEapMethodFastInfo &info);

    void getSecuType(KySecuType &secuType, KyEapMethodType &enterpriseType);
    bool getAutoConnectState();

private:
    bool isDetailPage;
//    QFormLayout *mSecuLayout;
    QGridLayout *topLayout;
    QGridLayout *bottomLayout;
    QVBoxLayout *mainLayout;

    QLabel *secuTypeLabel;
    QLabel *pwdLabel;
    //企业wifi共有
    QLabel *eapTypeLabel;
    //TLS
    QLabel *identityLable;
    QLabel *domainLable;
    QLabel *caCertPathLabel;
    QLabel *caNeedFlagLabel;
    FixLabel *clientCertPathLabel;
    FixLabel *clientPrivateKeyLabel;
    FixLabel *clientPrivateKeyPwdLabel;
    FixLabel *pwdOptionLabel;

    //PEAP TTLS共有
    FixLabel *eapMethodLabel;
    QLabel *userNameLabel;
    QLabel *userPwdLabel;
    QLabel *userPwdFlagLabel;

    QComboBox *secuTypeCombox;
    KPasswordEdit *pwdEdit = nullptr;
    QComboBox *eapTypeCombox;
    //TLS
    LineEdit *identityEdit;
    LineEdit *domainEdit;
    QComboBox *caCertPathCombox;
    QCheckBox *caNeedBox;
    QComboBox *clientCertPathCombox;
    QComboBox *clientPrivateKeyCombox;
    KPasswordEdit *clientPrivateKeyPwdEdit = nullptr;
    QComboBox *pwdOptionCombox;
    QWidget *tlsWidget;

    //PEAP && TTLS
    QComboBox *eapMethodCombox;
    LineEdit *userNameEdit;
    KPasswordEdit *userPwdEdit = nullptr;
    QCheckBox *userPwdFlagBox;

    //FAST
    QCheckBox *m_pacCheckBox;
    QComboBox *m_pacProvisionComboBox;
    QComboBox *m_pacFilePathComboBox;
    FixLabel *m_pacProvisionLabel;
    FixLabel *m_pacFlagLabel;
    QLabel *m_pacFileLabel;
    QWidget *m_pacCheckWidget;

    QLabel *m_emptyLabel = nullptr;
    QLabel *m_checkLabel = nullptr;
    QCheckBox *m_rememberCheckBox = nullptr;

    QString hintRequired = tr("Required"); //必填
    QString emptyhint = tr(" ");

private:
    void showNone();
    void showPsk();
    void showTls();
    void showPeapOrTtls();
    void showLeapOrPwd();
    void showFast();
    void initUI();
    void initConnect();

    KyEapMethodTlsInfo assembleTlsInfo();
    KyEapMethodPeapInfo assemblePeapInfo();
    KyEapMethodTtlsInfo assembleTtlsInfo();
    KyEapMethodLeapInfo assembleLeapInfo();
    KyEapMethodPwdInfo assemblePwdInfo();
    KyEapMethodFastInfo assembleFastInfo();

    bool checkConnectBtnIsEnabled();

private Q_SLOTS:
    void onSecuTypeComboxIndexChanged();
    void onEapTypeComboxIndexChanged();
    void setEnableOfSaveBtn();

    void onCaNeedBoxClicked();
    void onPacBoxClicked();

    void onCaCertPathComboxIndexChanged(QString str);
    void onClientCertPathComboxIndexChanged(QString str);
    void onClientPrivateKeyComboxIndexChanged(QString str);
    void onPwdOptionComboxIndexChanged(QString str);
    void changeColumnWidthWithSecuType();
    void onPacFilePathComboxIndexChanged(QString str);

Q_SIGNALS:
    void setSecuPageState(bool);
    void secuTypeChanged(const KySecuType &type);
    void eapTypeChanged(const KyEapMethodType &type);
};

#endif // SECURITYWIDGET_H
