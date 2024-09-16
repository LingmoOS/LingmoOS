// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "utils.h"
#include "userdialog.h"
#include "trans_string.h"

#include <DCommandLinkButton>
#include <DPasswordEdit>
#include <DSuggestButton>
#include <DFontSizeManager>
#include <DLabel>
#include <DFrame>
#include <DGuiApplicationHelper>

#include <QDebug>
#include <QTimer>
#include <QRegExpValidator>
#include <QNetworkConfigurationManager>
#include <QRegularExpression>

static const QString STRING_ICONUOSID = QStringLiteral("dcc_union_id");
static const QString STRING_PWDREG = QStringLiteral("^(\\w|\\+|\\[|\\]|-|[~`!@#$%^&()*={}:\"<>,.?|/])+$");


static QString CheckPassword(const QString &pwd)
{
    QString strerr;
    if(!utils::isContainDigitAndChar(pwd)) {
        return TransString::getTransString(STRING_PWDDIGCHAR);
    }

    if(pwd.length() < 8) {
        return TransString::getTransString(STRING_PWDLENGTH);
    }

    return strerr;
}

VerifyDlg::VerifyDlg(QWidget *parent):DDialog (parent)
{
    setTitle(TransString::getTransString(STRING_VERIFYTITLE));
    setMessage(TransString::getTransString(STRING_VERIFYMSG));
    m_pwdEdit = new DPasswordEdit;
    m_pwdEdit->setPlaceholderText(TransString::getTransString(STRING_PWDHOLDER));
    m_pwdEdit->lineEdit()->setMaxLength(64);
    m_pwdEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp(STRING_PWDREG), this));

    DCommandLinkButton *passwdBtn = new DCommandLinkButton(TransString::getTransString(STRING_FORGETPWD), this);
    DFontSizeManager::instance()->bind(passwdBtn, DFontSizeManager::T7);
    setFixedWidth(400);
    setIcon(QIcon::fromTheme(STRING_ICONUOSID));
    setSpacing(0);
    addSpacing(20);
    addContent(m_pwdEdit);
    addSpacing(8);
    addContent(passwdBtn, Qt::AlignRight);
    addButton(TransString::getTransString(STRING_CANCEL));
    addButton(TransString::getTransString(STRING_CONFIRM), false, DDialog::ButtonRecommend);
    setOnButtonClickedClose(false);
    setDisplayPosition(Dtk::Widget::DAbstractDialog::Center);
    //
    connect(m_pwdEdit, &DLineEdit::editingFinished, [=]{
        m_pwdEdit->setAlert(false);
        m_pwdEdit->hideAlertMessage();
    });
    connect(passwdBtn, &QAbstractButton::clicked, [this]{
        Q_EMIT this->forgetPasswd();
    });
    connect(getButton(0), &QAbstractButton::clicked, this, &QDialog::reject);
    connect(getButton(1), &QAbstractButton::clicked, [=]{
        QString strpwd = m_pwdEdit->text();
        QString checkResult = CheckPassword(strpwd);
        if(!checkResult.isEmpty()) {
            m_pwdEdit->setAlert(true);
            m_pwdEdit->showAlertMessage(checkResult);
            return;
        }

        if(!m_pwdEdit->lineEdit()->hasAcceptableInput()) {
            m_pwdEdit->setAlert(true);
            m_pwdEdit->showAlertMessage(TransString::getTransString(STRING_PWDINVALID));
            return;
        }

        Q_EMIT verifyPasswd(strpwd);
    });
    m_pwdEdit->setFocus();
}

void VerifyDlg::showAlert(const QString &alertMsg)
{
    m_pwdEdit->setAlert(true);
    m_pwdEdit->showAlertMessage(alertMsg);
}

PhoneMailDlg::PhoneMailDlg(QWidget *parent):DDialog (parent)
{
    QMargins contentMargin(10, 0, 10, 0);
    setIcon(QIcon::fromTheme(STRING_ICONUOSID));
    setFixedWidth(400);
    QWidget *phoneWidget = new QWidget;
    QVBoxLayout *phLayout1 = new QVBoxLayout;
    QHBoxLayout *phLayout2 = new QHBoxLayout;
    m_numEdit = new DLineEdit;
    m_codeEdit = new DLineEdit;
    m_numEdit->setFixedHeight(36);
    m_codeEdit->setFixedSize(237, 36);
    m_btnCode = new DSuggestButton(TransString::getTransString(STRING_VERIFYCODE));
    m_btnCode->setFixedHeight(36);
    m_btnCode->setFixedWidth(132);
    m_btnCode->setEnabled(false);
    m_codeEdit->lineEdit()->setMaxLength(6);
    m_codeEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("^[0-9]{6}$"), this));

    m_codeEdit->setPlaceholderText(TransString::getTransString(STRING_VERIFYHOLDER));
    phLayout2->setContentsMargins(0, 0, 0, 0);
    phLayout2->addWidget(m_codeEdit, 0, Qt::AlignLeft);
    phLayout2->addWidget(m_btnCode, 0, Qt::AlignRight);
    phLayout1->setSpacing(10);
    phLayout2->setSpacing(10);
    phLayout1->setContentsMargins(0, 0, 0, 0);
    phLayout1->addWidget(m_numEdit);
    phLayout1->addLayout(phLayout2);
    phoneWidget->setLayout(phLayout1);
    phoneWidget->setContentsMargins(0, 0, 0, 0);
    setContentLayoutContentsMargins(contentMargin);
    setSpacing(0);
    addSpacing(16);
    addContent(phoneWidget);
    addButton(TransString::getTransString(STRING_CANCEL));
    addButton(TransString::getTransString(STRING_CONFIRM), true, DDialog::ButtonRecommend);
    setOnButtonClickedClose(false);
    //
    connect(m_numEdit, &DLineEdit::editingFinished, [=]{
        m_numEdit->setAlert(false);
        m_numEdit->hideAlertMessage();
    });
    connect(m_numEdit, &DLineEdit::textChanged, [=]{
        if(!m_numEdit->text().isEmpty() && m_numEdit->lineEdit()->hasAcceptableInput()) {
            m_btnCode->setEnabled(true);
        } else {
            m_btnCode->setEnabled(false);
        }
    });
    connect(m_codeEdit, &DLineEdit::editingFinished, [=]{
        m_codeEdit->setAlert(false);
        m_codeEdit->hideAlertMessage();
    });
    connect(getButton(0), &QAbstractButton::clicked, this, &QDialog::reject);
    connect(m_btnCode, &QPushButton::clicked, [=]{
        QString strNum = m_numEdit->text();
        if(strNum.isEmpty() || !m_numEdit->lineEdit()->hasAcceptableInput())
        {
            m_numEdit->setAlert(true);
            m_numEdit->showAlertMessage(m_invalidTip);
            return;
        }

        Q_EMIT sendVerify(strNum);
    });

    connect(getButton(1), &QAbstractButton::clicked, [=]{
        QString strNum = m_numEdit->text();
        QString verifyCode = m_codeEdit->text();
        if(!m_numEdit->lineEdit()->hasAcceptableInput())
        {
            m_numEdit->setAlert(true);
            m_numEdit->showAlertMessage(m_invalidTip);
            return;
        }

        if(!m_codeEdit->lineEdit()->hasAcceptableInput())
        {
            m_codeEdit->setAlert(true);
            m_codeEdit->showAlertMessage(TransString::getTransString(STRING_VERIFYINVALID));
            return;
        }

        Q_EMIT updatePhoneMail(strNum, verifyCode);
    });
    m_numEdit->setFocus();
}

void PhoneMailDlg::setNumHolderText(const QString &holderText)
{
    m_numEdit->setPlaceholderText(holderText);
}

void PhoneMailDlg::setInvalidTip(const QString &invalidTip)
{
    m_invalidTip = invalidTip;
}

void PhoneMailDlg::initPhoneNumConstraint()
{
    m_numEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("^1[3-9]\\d{9}$"), this));
}

void PhoneMailDlg::initMailNumConstraint()
{
    m_numEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("^.+@.+\\..+$"), this));
}

void PhoneMailDlg::showVerifyAlert()
{
    m_codeEdit->setAlert(true);
    m_codeEdit->showAlertMessage(TransString::getTransString(STRING_FAILTIP));
}

void PhoneMailDlg::verifyCoolDown(int seconds)
{
    m_coolDown = seconds - 1;
    m_btnCode->setEnabled(false);
    QTimer *timerVerify = new QTimer(this);
    timerVerify->setInterval(1000);
    connect(timerVerify, &QTimer::timeout, [=]{
        if(m_coolDown == 0) {
            timerVerify->deleteLater();
            m_btnCode->setEnabled(true);
            m_btnCode->setText(TransString::getTransString(STRING_VERIFYCODE));
            return;
        }

        m_btnCode->setText(TransString::getTransString(STRING_RESENDCODE).arg(m_coolDown--));
    });
    timerVerify->start();
}

void PhoneMailDlg::bindToLocal()
{
    QString strnum = m_numEdit->text();
    QString verify = m_codeEdit->text();
    Q_EMIT rebindPhoneMail(strnum, verify, m_rebindKey);
}

ResetPwdDlg::ResetPwdDlg(QWidget *parent):DDialog (parent)
{
    setTitle(TransString::getTransString(STRING_RESETPWDTITLE));
    setIcon(QIcon::fromTheme(STRING_ICONUOSID));
    setFixedWidth(400);
    setSpacing(0);
    m_pwdEdit = new DPasswordEdit;
    m_pwdEdit->lineEdit()->setMaxLength(64);
    m_pwdEdit->setPlaceholderText(TransString::getTransString(STRING_PWDHOLDER));
    m_pwdEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp(STRING_PWDREG), this));
    m_againEdit = new DPasswordEdit;
    m_againEdit->lineEdit()->setMaxLength(64);
    m_againEdit->setPlaceholderText(TransString::getTransString(STRING_AGAINHOLDER));
    m_againEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp(STRING_PWDREG), this));
    addSpacing(20);
    addContent(m_pwdEdit);
    addSpacing(10);
    addContent(m_againEdit);
    addButton(TransString::getTransString(STRING_CANCEL));
    addButton(TransString::getTransString(STRING_CONFIRM), true, DDialog::ButtonRecommend);
    setOnButtonClickedClose(false);
    connect(getButton(0), &QAbstractButton::clicked, this, &QDialog::reject);
    connect(m_pwdEdit, &DLineEdit::editingFinished, [=]{
        m_pwdEdit->setAlert(false);
        m_pwdEdit->hideAlertMessage();
    });
    connect(m_againEdit, &DLineEdit::editingFinished, this, [=]{
        QString strpwd = m_pwdEdit->text();
        QString strVerify = m_againEdit->text();
        if(strpwd != strVerify){
            m_againEdit->setAlert(true);
            m_againEdit->showAlertMessage(TransString::getTransString(STRING_PWDNOTMATCH));
        }
        else {
            m_againEdit->setAlert(false);
            m_againEdit->hideAlertMessage();
        }
    });
    connect(getButton(1), &QAbstractButton::clicked, [=]{
        QString strpwd = m_pwdEdit->text();
        QString strVerify = m_againEdit->text();
        QString checkResult = CheckPassword(strpwd);
        if(!checkResult.isEmpty()) {
            m_pwdEdit->setAlert(true);
            m_pwdEdit->showAlertMessage(checkResult);
            return;
        }

        QString checkAgain = CheckPassword(strVerify);
        if(!checkAgain.isEmpty()) {
            m_againEdit->setAlert(true);
            m_againEdit->showAlertMessage(checkAgain);
            return;
        }

        if(!m_pwdEdit->lineEdit()->hasAcceptableInput()) {
            m_pwdEdit->setAlert(true);
            m_pwdEdit->showAlertMessage(TransString::getTransString(STRING_PWDINVALID));
            return;
        }

        if(strpwd != strVerify) {
            m_againEdit->setAlert(true);
            m_againEdit->showAlertMessage(TransString::getTransString(STRING_PWDNOTMATCH));
            return;
        }
        else {
            Q_EMIT this->resetPasswd(strpwd);
        }
    });
    m_pwdEdit->setFocus();
}

void ResetPwdDlg::showFailTip()
{
    m_pwdEdit->setAlert(true);
    m_pwdEdit->showAlertMessage(TransString::getTransString(STRING_FAILTIP));
}

RegisterDlg::RegisterDlg(QWidget *parent):DDialog (parent)
{
    setTitle(TransString::getTransString(STRING_REGISTERPWD));
    setFixedWidth(400);
    setIcon(QIcon::fromTheme(STRING_ICONUOSID));
    setSpacing(0);
    m_pwdEdit = new DPasswordEdit;
    m_pwdEdit->lineEdit()->setMaxLength(64);
    m_pwdEdit->setPlaceholderText(TransString::getTransString(STRING_PWDHOLDER));
    m_pwdEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp(STRING_PWDREG), this));
    m_againEdit = new DPasswordEdit;
    m_againEdit->lineEdit()->setMaxLength(64);
    m_againEdit->setPlaceholderText(TransString::getTransString(STRING_AGAINHOLDER));
    m_againEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp(STRING_PWDREG), this));
    addSpacing(20);
    addContent(m_pwdEdit);
    addSpacing(10);
    addContent(m_againEdit);
    addButton(TransString::getTransString(STRING_CANCEL));
    addButton(TransString::getTransString(STRING_CONFIRM), true, DDialog::ButtonRecommend);
    setOnButtonClickedClose(false);
    connect(getButton(0), &QAbstractButton::clicked, this, &QDialog::reject);
    connect(m_pwdEdit, &DLineEdit::editingFinished, [=]{
        m_pwdEdit->setAlert(false);
        m_pwdEdit->hideAlertMessage();
    });
    connect(m_againEdit, &DLineEdit::editingFinished, [=]{
        QString strpwd = m_pwdEdit->text();
        QString strVerify = m_againEdit->text();
        if(strpwd != strVerify){
            m_againEdit->setAlert(true);
            m_againEdit->showAlertMessage(TransString::getTransString(STRING_PWDNOTMATCH));
        }
        else {
            m_againEdit->setAlert(false);
            m_againEdit->hideAlertMessage();
        }
    });
    connect(getButton(1), &QAbstractButton::clicked, [=]{
        QString strpwd = m_pwdEdit->text();
        QString strpwdCheck = m_againEdit->text();
        QString checkResult = CheckPassword(strpwd);
        if(!checkResult.isEmpty()) {
            m_pwdEdit->setAlert(true);
            m_pwdEdit->showAlertMessage(checkResult);
            return;
        }

        QString checkAgain = CheckPassword(strpwdCheck);
        if(!checkAgain.isEmpty()) {
            m_againEdit->setAlert(true);
            m_againEdit->showAlertMessage(checkAgain);
            return;
        }

        if(!m_pwdEdit->lineEdit()->hasAcceptableInput()) {
            m_pwdEdit->setAlert(true);
            m_pwdEdit->showAlertMessage(TransString::getTransString(STRING_PWDINVALID));
            return;
        }

        if(strpwd != strpwdCheck){
            //tip passwd is not the same
            m_againEdit->setAlert(true);
            m_againEdit->showAlertMessage(TransString::getTransString(STRING_PWDNOTMATCH));
        }
        else {
            Q_EMIT this->registerPasswd(strpwd);
        }
    });
    m_pwdEdit->setFocus();
}

ReBindDlg::ReBindDlg(QWidget *parent):DDialog (parent)
{
    setFixedSize(400, 226);
    setIcon(QIcon::fromTheme(STRING_ICONUOSID));
    addButton(TransString::getTransString(STRING_BINDLOCAL));
    addButton("", true, DDialog::ButtonRecommend);
    setOnButtonClickedClose(false);
    connect(getButton(0), &QAbstractButton::clicked, [this]{
        this->m_changeNum = false;
        this->accept();
    });
    connect(getButton(1), &QAbstractButton::clicked, [this]{
        this->m_changeNum = true;
        this->accept();
    });
}

void ReBindDlg::setTitleTip(const QString &strtitle)
{
    setTitle(strtitle);
}

void ReBindDlg::setChangeTip(const QString &strChange)
{
    getButton(1)->setText(strChange);
}

void ReBindDlg::setDialogMessage(const QString &msg)
{
    setMessage(msg);
}
