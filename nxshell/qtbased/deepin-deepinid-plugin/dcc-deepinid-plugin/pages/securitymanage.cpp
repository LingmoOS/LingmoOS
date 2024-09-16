// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "securitymanage.h"
#include "utils.h"
#include "trans_string.h"
#include "singleitem.h"

#include <DLabel>
#include <DTipLabel>

#include <DListView>
#include <DDialog>
#include <DPasswordEdit>
#include <DSuggestButton>
#include <DFontSizeManager>
#include <DFrame>
#include <DGuiApplicationHelper>
#include <DBackgroundGroup>

#include <QVBoxLayout>
#include <QTimer>

DWIDGET_USE_NAMESPACE
Q_DECLARE_METATYPE(QMargins)

static const QString STRING_ICONUOSID = QStringLiteral("dcc_union_id");
static const QString CLIENT_SERVICE = QStringLiteral("com.deepin.deepinid.Client");
static const QString CLIENT_PATH = QStringLiteral("/com/deepin/deepinid/Client");

SecurityPage::SecurityPage(QWidget *parent) : QWidget(parent)
  , m_unbindWeChatDlg(new DDialog("", "", this))
  , m_phoneWidget(new QWidget(this))
  , m_accountWidget(new QWidget(this))
  , m_passwdWidget(new QWidget(this))
  , m_itemPhone(new SingleItem)
  , m_itemMail(new SingleItem)
  , m_itemAccount(new SingleItem)
  , m_itemPasswd(new SingleItem)
  , m_clientService(nullptr)
{
    m_forgetUrl = utils::forgetPwdURL();
    //qDebug() << "forget url:" << m_forgetUrl;
    m_wechatUrl = utils::wechatURL();
    //qDebug() << "wechat url:" << m_wechatUrl;

    initUI();
    initDialog();
    initConnection();
    initModelData();
}

void SecurityPage::setSyncModel(SyncModel *syncModel)
{
    m_syncModel = syncModel;
    connect(m_syncModel, &SyncModel::userInfoChanged, this, &SecurityPage::onUserInfoChanged);
}

void SecurityPage::setSyncWorker(SyncWorker *worker)
{
    m_syncWorker = worker;
}

void SecurityPage::onLogin()
{
    //init data
    onUserInfoChanged(m_syncModel->userinfo());
}

void SecurityPage::initUI()
{
    QVBoxLayout *mainlayout = new QVBoxLayout;
    //head
    QVBoxLayout *headlayout = new QVBoxLayout;
    headlayout->setContentsMargins(0, 5, 0, 0);
    headlayout->setSpacing(10);
    //phone group
    QVBoxLayout *bgphone = new QVBoxLayout;
    bgphone->setContentsMargins(0, 0, 0, 0);
    DBackgroundGroup *phoneGroup = new DBackgroundGroup(bgphone, m_phoneWidget);
    phoneGroup->setItemSpacing(1);
    phoneGroup->setItemMargins(QMargins(0, 0, 0, 0));
    bgphone->addWidget(m_itemPhone);
    bgphone->addWidget(m_itemMail);
    phoneGroup->setBackgroundRole(QPalette::Window);
    phoneGroup->setUseWidgetBackground(false);
    QHBoxLayout *phonebgLayout = new QHBoxLayout;
    phonebgLayout->setContentsMargins(0, 0, 0, 0);
    m_phoneWidget->setLayout(phonebgLayout);
    phonebgLayout->addWidget(phoneGroup);
    m_phoneWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    DLabel *phone = new DLabel(TransString::getTransString(STRING_TITLEPHONE), this);
    DTipLabel *phoneTip = new DTipLabel(TransString::getTransString(STRING_MSGPHONE), this);
    phoneTip->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    phoneTip->setWordWrap(true);
    QHBoxLayout *tiplayout = new QHBoxLayout();
    tiplayout->setContentsMargins(0, 0, 0, 0);
    tiplayout->addWidget(phoneTip);
    phone->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    phone->setWordWrap(true);
    QHBoxLayout *phoneLayout = new QHBoxLayout;
    phoneLayout->setContentsMargins(0, 0, 0, 0);
    phoneLayout->addWidget(phone);
    headlayout->addLayout(phoneLayout);
    headlayout->addSpacing(-5);
    headlayout->addLayout(tiplayout);
    headlayout->addWidget(m_phoneWidget);

    //bind
    QVBoxLayout *bindlayout = new QVBoxLayout;
    bindlayout->setSpacing(10);
    DLabel *bindThird = new DLabel(TransString::getTransString(STRING_WECHATTITLE), this);
    DTipLabel *bindThirdTip = new DTipLabel(TransString::getTransString(STRING_WECHATMSG), this);
    bindThirdTip->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    bindThirdTip->setWordWrap(true);
    //bind group
    QVBoxLayout *bgaccount = new QVBoxLayout;
    bgaccount->setContentsMargins(0, 0, 0, 0);
    DBackgroundGroup *accountGroup = new DBackgroundGroup(bgaccount, m_accountWidget);
    accountGroup->setItemSpacing(1);
    accountGroup->setItemMargins(QMargins(0, 0, 0, 0));
    bgaccount->addWidget(m_itemAccount);
    accountGroup->setBackgroundRole(QPalette::Window);
    accountGroup->setUseWidgetBackground(false);
    QHBoxLayout *accountbgLayout = new QHBoxLayout;
    accountbgLayout->setContentsMargins(0, 0, 0, 0);
    m_accountWidget->setLayout(accountbgLayout);
    accountbgLayout->addWidget(accountGroup);
    m_accountWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *acctipLayout = new QHBoxLayout;
    acctipLayout->setContentsMargins(0, 0, 0, 0);
    acctipLayout->addWidget(bindThirdTip);
    bindlayout->addWidget(bindThird, 0, Qt::AlignLeft);
    bindlayout->addSpacing(-5);
    bindlayout->addLayout(acctipLayout);
    bindlayout->addWidget(m_accountWidget);

    //password
    QVBoxLayout *passwdlayout = new QVBoxLayout;
    passwdlayout->setSpacing(10);
    DLabel *passwd = new DLabel(TransString::getTransString(STRING_PASSWORDTITLE), this);
    DTipLabel *passwdTip = new DTipLabel(TransString::getTransString(STRING_PASSWORDMSG), this);
    //password group
    QVBoxLayout *bgpasswd = new QVBoxLayout;
    bgpasswd->setContentsMargins(0, 0, 0, 0);
    DBackgroundGroup *passwdGroup = new DBackgroundGroup(bgpasswd, m_passwdWidget);
    passwdGroup->setItemSpacing(1);
    passwdGroup->setItemMargins(QMargins(0, 0, 0, 0));
    bgpasswd->addWidget(m_itemPasswd);
    passwdGroup->setBackgroundRole(QPalette::Window);
    passwdGroup->setUseWidgetBackground(false);
    QHBoxLayout *passwdbgLayout = new QHBoxLayout;
    passwdbgLayout->setContentsMargins(0, 0, 0, 0);
    m_passwdWidget->setLayout(passwdbgLayout);
    passwdbgLayout->addWidget(passwdGroup);
    m_passwdWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    passwdlayout->addWidget(passwd, 0, Qt::AlignLeft);
    passwdlayout->addSpacing(-5);
    passwdlayout->addWidget(passwdTip, 0, Qt::AlignLeft);
    passwdlayout->addWidget(m_passwdWidget);
    //
    DFontSizeManager::instance()->bind(phone, DFontSizeManager::T5, QFont::DemiBold);
    DFontSizeManager::instance()->bind(phoneTip, DFontSizeManager::T8, QFont::Thin);
    DFontSizeManager::instance()->bind(bindThird, DFontSizeManager::T5, QFont::DemiBold);
    DFontSizeManager::instance()->bind(bindThirdTip, DFontSizeManager::T8, QFont::Thin);
    DFontSizeManager::instance()->bind(passwd, DFontSizeManager::T5, QFont::DemiBold);
    DFontSizeManager::instance()->bind(passwdTip, DFontSizeManager::T8, QFont::Thin);

    mainlayout->setMargin(20);
    mainlayout->setSpacing(20);
    mainlayout->addLayout(headlayout);
    mainlayout->addLayout(bindlayout);
    mainlayout->addLayout(passwdlayout);
    mainlayout->addStretch();
    setLayout(mainlayout);
}

void SecurityPage::initConnection()
{
    connect(this, &SecurityPage::onUserLogout, m_unbindWeChatDlg, &QDialog::reject);
    connect(m_itemPasswd, &SingleItem::clicked, [=]{
        emit this->onChangeInfo();
    });
}

void SecurityPage::initModelData()
{
    m_itemPhone->SetIcon("dcc_trust_phone");
    m_itemMail->SetIcon("dcc_trust_email");
    m_itemAccount->SetIcon("dcc_secwechat");
    m_itemPasswd->SetIcon("dcc_seckey");
    m_itemPasswd->SetText(TransString::getTransString(STRING_PASSWORDTITLE));
    m_itemPasswd->SetLinkText(TransString::getTransString(STRING_PWDACTION));
}

void SecurityPage::initDialog()
{
    //m_unbindWeChatDlg->setFixedWidth(500);
    //m_unbindWeChatDlg->setMinimumHeight(226);
    m_unbindWeChatDlg->setIcon(QIcon::fromTheme(STRING_ICONUOSID));
    m_unbindWeChatDlg->setTitle(TransString::getTransString(STRING_WECHATUNBINDMESSAGE));
    m_unbindWeChatDlg->setMessage(TransString::getTransString(STRING_WECHATUNBINDTITLE));
    m_unbindWeChatDlg->addButton(TransString::getTransString(STRING_CANCEL));
    m_unbindWeChatDlg->addButton(TransString::getTransString(STRING_UNBINDTIP), true, DDialog::ButtonWarning);
}

void SecurityPage::initRegisterDialog(RegisterDlg *dlg)
{
    connect(dlg, &RegisterDlg::registerPasswd, this, [=](const QString &strpwd){
        m_syncWorker->registerPasswd(strpwd);
        dlg->accept();
    });
    connect(this, &SecurityPage::onUserLogout, dlg, &QDialog::reject);
}

void SecurityPage::initVerifyDialog(VerifyDlg *dlg)
{
    connect(dlg, &VerifyDlg::verifyPasswd, [=](const QString &strpwd){
        QString encryptPwd;
        int remainNum = 0;
        if(m_syncWorker->checkPassword(strpwd, encryptPwd, remainNum))
        {
            qInfo() << __LINE__ << "check password success";
            m_lastEnPwd = encryptPwd;
            dlg->accept();
        }
        else
        {
            qInfo() << __LINE__ << "check password failed:";
            //tip passwd is wrong
            dlg->showAlert(utils::getRemainPasswdMsg(remainNum));
        }
    });
    connect(dlg, &VerifyDlg::forgetPasswd, [this]{
        m_syncWorker->openForgetPasswd(QString("%1&time=%2").arg(m_forgetUrl).arg(QDateTime::currentMSecsSinceEpoch()));
    });
    connect(this, &SecurityPage::onUserLogout, dlg, &QDialog::reject);
}

void SecurityPage::initPhoneDialog(PhoneMailDlg *dlg)
{
    qInfo() << "init phone dialog";
    dlg->setTitle(TransString::getTransString(STRING_PHONEBIND));
    dlg->initPhoneNumConstraint();
    dlg->setNumHolderText(TransString::getTransString(STRING_PHONEHOLDER));
    dlg->setInvalidTip(TransString::getTransString(STRING_PHONEINVALID));
    initPhoneMailConnection(dlg);
}

void SecurityPage::initMailDialog(PhoneMailDlg *dlg)
{
    qInfo() << "init mail dialog";
    dlg->setTitle(TransString::getTransString(STRING_MAILBIND));
    dlg->initMailNumConstraint();
    dlg->setNumHolderText(TransString::getTransString(STRING_MAILHOLDER));
    dlg->setInvalidTip(TransString::getTransString(STRING_MAILINVALID));
    initPhoneMailConnection(dlg);
}

void SecurityPage::initPhoneMailConnection(PhoneMailDlg *dlg)
{
    connect(dlg, &PhoneMailDlg::sendVerify, [=](const QString &strNum){
        int countDown = m_syncWorker->sendVerifyCode(strNum);
        if(countDown > 0) {
            dlg->verifyCoolDown(countDown);
        }
        else {
            dlg->showVerifyAlert();
        }
    });
    connect(dlg, &PhoneMailDlg::updatePhoneMail, [=](const QString &strnum, const QString &verify){
        QString rebindAccount;
        QString rebindKey;
        //qDebug() << "update phonemail:" << strnum << verify;
        bool ret = m_syncWorker->updatePhoneEmail(strnum, verify, rebindAccount, rebindKey);
        if(!ret)
        {
            if(rebindAccount.isEmpty()) {
                qDebug() << "show fail";
                utils::sendSysNotify(TransString::getTransString(STRING_FAILTIP));
            }
            else {
                qDebug() << "show rebind dialog:" << rebindAccount << rebindKey;
                dlg->setRebindInfo(rebindAccount, rebindKey);
            }
        }
        else
        {
            utils::sendSysNotify(TransString::getTransString(STRING_SUCCESSTIP));
        }

        dlg->accept();
    });
    connect(dlg, &PhoneMailDlg::rebindPhoneMail, [=](const QString &strnum, const QString &verify, const QString &key){
        qInfo() << strnum << verify << key;
        QString rebindAccount;
        QString rebindKey = key;
        bool ret = m_syncWorker->updatePhoneEmail(strnum, verify, rebindAccount, rebindKey);
        if(!ret)
        {
            utils::sendSysNotify(TransString::getTransString(STRING_FAILTIP));
        }
        else
        {
            utils::sendSysNotify(TransString::getTransString(STRING_SUCCESSTIP));
        }
    });
    connect(this, &SecurityPage::onUserLogout, dlg, &QDialog::reject);
}

void SecurityPage::initResetPwdDialog(ResetPwdDlg *dlg)
{
    connect(dlg, &ResetPwdDlg::resetPasswd, this, [=](const QString &newpwd){
        if(m_syncWorker->resetPassword(m_lastEnPwd, newpwd)) {
            utils::sendSysNotify(TransString::getTransString(STRING_RESETPWDTIP));
            dlg->accept();
        }
        else {
            dlg->showFailTip();
        }
    });
    connect(this, &SecurityPage::onUserLogout, dlg, &QDialog::reject);
}

void SecurityPage::onBindSuccess()
{
    qInfo() << "on bind success";
    utils::sendSysNotify(TransString::getTransString(STRING_SUCCESSTIP));
    m_syncWorker->refreshUserInfo();
}

void SecurityPage::onUserInfoChanged(const QVariantMap &infos)
{
    QString phone = infos["Phone"].toString();
    QString mail = infos["Email"].toString();
    // TODO 这里有个遗留BUG，通过小程序注册的账户没有微信昵称，会显示为未绑定
    QString wechat = infos["WechatNickname"].toString();
    m_itemPhone->SetText(phone.trimmed().isEmpty() ? TransString::getTransString(STRING_UNLINKED) : phone.trimmed());

    m_itemMail->SetText(mail.trimmed().isEmpty() ? TransString::getTransString(STRING_UNLINKED) : mail.trimmed());

    m_itemAccount->SetText(wechat.trimmed().isEmpty() ? TransString::getTransString(STRING_UNLINKED) : wechat.trimmed());
}

bool SecurityPage::verifyPasswd(VerifyType type)
{
    int ret;
    //check password is empty
    bool isEmpty = false;
    if(!m_syncWorker->checkPasswdEmpty(isEmpty))
    {
        qWarning() << "check password empty failed";
        return false;
    }

    if(isEmpty)
    {
        //password is empty
        RegisterDlg regDlg;
        initRegisterDialog(&regDlg);
        ret = regDlg.exec();
    }
    else
    {
        VerifyDlg verifyDlg;
        initVerifyDialog(&verifyDlg);
        ret = verifyDlg.exec();
    }

    qDebug() << __LINE__ << type << ", verify pwd ret:" << (ret == QDialog::Accepted);
    if(ret == QDialog::Accepted)
    {
        if(type != PasswdType || !isEmpty)
        {
            openUserDialog(type);
        }
    }

    return true;
}

void SecurityPage::openUserDialog(SecurityPage::VerifyType type)
{
    switch (type)
    {
    case PhoneType:
    {
        bool bFinish = true;
        do
        {
            PhoneMailDlg phoneDlg;
            initPhoneDialog(&phoneDlg);
            phoneDlg.exec();
            qInfo() << "phone update done";
            if(!phoneDlg.rebindAccount().isEmpty())
            {
                ReBindDlg rebindDlg;
                rebindDlg.setTitleTip(TransString::getTransString(STRING_PHONEBIND));
                rebindDlg.setChangeTip(TransString::getTransString(STRING_USEANOPHONE));
                rebindDlg.setDialogMessage(TransString::getTransString(STRING_REBINDPHONE).arg(phoneDlg.rebindAccount()));
                connect(this, &SecurityPage::onUserLogout, &rebindDlg, &QDialog::reject);
                rebindDlg.exec();
                if(rebindDlg.result() == QDialog::Accepted)
                {
                    if(rebindDlg.isChangeNum()) {
                        bFinish = false;
                        continue;
                    }
                    else {
                        phoneDlg.bindToLocal();
                    }
                }
            }

            bFinish = true;
        }while (!bFinish);
    }
        break;
    case MailType:
    {
        bool bFinish = true;
        do
        {
            PhoneMailDlg mailDlg;
            initMailDialog(&mailDlg);
            mailDlg.exec();
            qInfo() << "mail update done";
            if(!mailDlg.rebindAccount().isEmpty())
            {
                ReBindDlg rebindDlg;
                rebindDlg.setTitleTip(TransString::getTransString(STRING_MAILBIND));
                rebindDlg.setChangeTip(TransString::getTransString(STRING_USEANOMAIL));
                rebindDlg.setDialogMessage(TransString::getTransString(STRING_REBINDMAIL).arg(mailDlg.rebindAccount()));
                connect(this, &SecurityPage::onUserLogout, &rebindDlg, &QDialog::reject);
                rebindDlg.exec();
                if(rebindDlg.result() == QDialog::Accepted)
                {
                    if(rebindDlg.isChangeNum()) {
                        bFinish = false;
                        continue;
                    }
                    else {
                        mailDlg.bindToLocal();
                    }
                }
            }

            bFinish = true;
        }while (!bFinish);
    }
        break;
    case BindAccountType:
    {
        QString bindurl;
        QString strsession = m_syncWorker->getSessionID();
        if(strsession.isEmpty()) {
            bindurl = "";
        }
        else {
            bindurl = m_wechatUrl;
            bindurl += "&sessionid=";
            bindurl += strsession;
            bindurl += QString("&time=%1").arg(QDateTime::currentMSecsSinceEpoch());
        }

        qDebug() << "Init wechat dialog, set bind url: " << bindurl;
        if(m_clientService == nullptr) {
            m_clientService = new QDBusInterface(CLIENT_SERVICE, CLIENT_PATH, CLIENT_SERVICE, QDBusConnection::sessionBus());
        } else if (!m_clientService->isValid()) {
            delete  m_clientService;
            m_clientService = new QDBusInterface(CLIENT_SERVICE, CLIENT_PATH, CLIENT_SERVICE, QDBusConnection::sessionBus());
        }

        connect(m_clientService, SIGNAL(bindSuccess()), this, SLOT(onBindSuccess()), Qt::UniqueConnection);
        m_clientService->asyncCall("BindAccount", bindurl);
    }
        break;
    case UnbindAccountType:
    {
        m_syncWorker->unBindPlatform();
        utils::sendSysNotify(TransString::getTransString(STRING_SUCCESSTIP));
    }
        break;
    case PasswdType:
    {
        ResetPwdDlg resetDlg;
        initResetPwdDialog(&resetDlg);
        resetDlg.exec();
    }
        break;
    }
}
