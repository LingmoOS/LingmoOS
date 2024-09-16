// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defsecuritytoolsfrontunit.h"

#include "securitytoolsmodel.h"
#include "src/window/modules/common/compixmap.h"

#include <DFloatingMessage>
#include <DMessageManager>
#include <DPalette>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QPixmap>
#include <QVBoxLayout>

DefSecurityToolsFrontUnit::DefSecurityToolsFrontUnit(DWidget *parent)
    : DFrame(parent)
    , m_toolInfo()
    , m_status(DEFSECURITYTOOLSTATUS::UNINSTALLED)
    , m_pImageLabel(nullptr)
    , m_pAppNameLabel(nullptr)
    , m_pDesLabel(nullptr)
    , m_pCompanyNameLabel(nullptr)
    , m_pOpenBtn(nullptr)
    , m_pUpdateBtn(nullptr)
    , m_pInstallBtn(nullptr)
    , m_pUnInstallBtn(nullptr)
    , m_pSpinner(nullptr)
    , m_pDataModel(nullptr)
{
    initUi();
    initTranslation();
}

DefSecurityToolsFrontUnit::~DefSecurityToolsFrontUnit()
{
    if (m_pImageLabel) {
        m_pImageLabel->deleteLater();
        m_pImageLabel = nullptr;
    }

    if (m_pAppNameLabel) {
        m_pAppNameLabel->deleteLater();
        m_pAppNameLabel = nullptr;
    }

    if (m_pDesLabel) {
        m_pDesLabel->deleteLater();
        m_pDesLabel = nullptr;
    }

    if (m_pCompanyNameLabel) {
        m_pCompanyNameLabel->deleteLater();
        m_pCompanyNameLabel = nullptr;
    }

    if (m_pOpenBtn) {
        m_pOpenBtn->deleteLater();
        m_pOpenBtn = nullptr;
    }

    if (m_pUpdateBtn) {
        m_pUpdateBtn->deleteLater();
        m_pUpdateBtn = nullptr;
    }

    if (m_pInstallBtn) {
        m_pInstallBtn->deleteLater();
        m_pInstallBtn = nullptr;
    }

    if (m_pUnInstallBtn) {
        m_pUnInstallBtn->deleteLater();
        m_pUnInstallBtn = nullptr;
    }
}

void DefSecurityToolsFrontUnit::setToolsInfo(const DEFSECURITYTOOLINFO &info)
{
    m_toolInfo = info;
    QPixmap pixmap = QIcon::fromTheme(info.strImagePath.trimmed()).pixmap(48, 48);
    m_pImageLabel->setPixmap(pixmap);
    setShowString(m_pAppNameLabel, getTranslation(info.strAppName), 140);
    setShowString(m_pDesLabel, getTranslation(info.strDescription), 140);
    setShowString(m_pCompanyNameLabel,
                  QString(tr("Supported by %1").arg(getTranslation(info.strCompany))),
                  140);

    if (info.bHasInstalled) {
        if (info.bCanUpdate) {
            setAppStatus(DEFSECURITYTOOLSTATUS::WAITFORUPDATE);
        } else {
            setAppStatus(DEFSECURITYTOOLSTATUS::INSTALLED);
        }
    } else {
        setAppStatus(DEFSECURITYTOOLSTATUS::UNINSTALLED);
    }

    if (info.bSystemTool) {
        m_pUnInstallBtn->setHidden(true);
    }
}

void DefSecurityToolsFrontUnit::setAppStatus(DEFSECURITYTOOLSTATUS status)
{
    m_status = status;

    switch (m_status) {
    case UNINSTALLED:
        m_pOpenBtn->setHidden(true);
        m_pUpdateBtn->setHidden(true);
        m_pUnInstallBtn->setHidden(true);
        m_pSpinner->setHidden(true);
        m_pSpinner->stop();
        m_pInstallBtn->setHidden(false);
        break;
    case INSTALLING:
        m_pOpenBtn->setHidden(true);
        m_pUpdateBtn->setHidden(true);
        m_pUnInstallBtn->setHidden(true);
        m_pInstallBtn->setHidden(true);
        m_pSpinner->setHidden(false);
        m_pSpinner->start();
        break;
    case INSTALLED:
        m_pOpenBtn->setHidden(false);
        m_pUpdateBtn->setHidden(true);
        m_pUnInstallBtn->setHidden(false);
        m_pInstallBtn->setHidden(true);
        m_pSpinner->setHidden(true);
        m_pSpinner->stop();
        break;
    case INSTALLFAILED:
        reportStatus(m_strInstallFailed);
        setAppStatus(DEFSECURITYTOOLSTATUS::UNINSTALLED);
        break;
    case INSTALLSUCCESS:
        setAppStatus(DEFSECURITYTOOLSTATUS::INSTALLED);
        break;
    case WAITFORUPDATE:
        m_pOpenBtn->setHidden(true);
        m_pUpdateBtn->setHidden(false);
        m_pUnInstallBtn->setHidden(false);
        m_pInstallBtn->setHidden(true);
        m_pSpinner->setHidden(true);
        break;
    case UPDATING:
        m_pOpenBtn->setHidden(true);
        m_pUpdateBtn->setHidden(true);
        m_pUnInstallBtn->setHidden(true);
        m_pInstallBtn->setHidden(true);
        m_pSpinner->setHidden(false);
        break;
    case UPDATEFAIL:
        reportStatus(m_strUpdateFailed);
        setAppStatus(DEFSECURITYTOOLSTATUS::WAITFORUPDATE);
        break;
    case UPDATASUCCESS:
        setAppStatus(DEFSECURITYTOOLSTATUS::INSTALLED);
        break;
    case UNINSTALLING:
        m_pOpenBtn->setHidden(true);
        m_pUpdateBtn->setHidden(true);
        m_pUnInstallBtn->setHidden(true);
        m_pInstallBtn->setHidden(true);
        m_pSpinner->setHidden(false);
        m_pSpinner->start();
        break;
    case UNINSTALLFAILED:
        reportStatus(m_strUninstallFailed);
        setAppStatus(DEFSECURITYTOOLSTATUS::INSTALLED);
        break;
    }
}

void DefSecurityToolsFrontUnit::setDataModel(QObject *pModel)
{
    Q_ASSERT((pModel != nullptr) && ("pModel can't be empty"));
    m_pDataModel = pModel;

    QObject::connect(m_pOpenBtn, &QPushButton::clicked, this, [=]() {
        this->showtool();
    });

    QObject::connect(m_pUpdateBtn, &QPushButton::clicked, this, [=]() {
        this->updatePackage();
    });

    QObject::connect(m_pInstallBtn, &QPushButton::clicked, this, [=]() {
        this->installPackage();
    });

    QObject::connect(m_pUnInstallBtn, &QPushButton::clicked, this, [=]() {
        this->unInstallPackage();
    });
}

void DefSecurityToolsFrontUnit::installPackage()
{
    SecurityToolsModel *pAttachedModel = qobject_cast<SecurityToolsModel *>(m_pDataModel);
    pAttachedModel->installTool(m_toolInfo.getPackageKey());
}

void DefSecurityToolsFrontUnit::unInstallPackage()
{
    SecurityToolsModel *pAttachedModel = qobject_cast<SecurityToolsModel *>(m_pDataModel);
    pAttachedModel->unInstallTool(m_toolInfo.getPackageKey());
}

void DefSecurityToolsFrontUnit::updatePackage()
{
    SecurityToolsModel *pAttachedModel = qobject_cast<SecurityToolsModel *>(m_pDataModel);
    pAttachedModel->updateTool(m_toolInfo.getPackageKey());
}

void DefSecurityToolsFrontUnit::showtool()
{
    SecurityToolsModel *pAttachedModel = qobject_cast<SecurityToolsModel *>(m_pDataModel);
    pAttachedModel->openTool(m_toolInfo.getPackageKey());
}

void DefSecurityToolsFrontUnit::updateAppStatus()
{
}

void DefSecurityToolsFrontUnit::initUi()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    pMainLayout->setContentsMargins(10, 10, 10, 10);
    pMainLayout->setSpacing(10);
    m_pImageLabel = new DLabel;
    QPixmap pixMap = QPixmap(48, 48);
    pixMap.fill(Qt::darkGray);
    m_pImageLabel->setPixmap(pixMap);

    QVBoxLayout *pVlayout = new QVBoxLayout;
    pVlayout->setSpacing(2);
    m_pAppNameLabel = createLabel(14);
    m_pAppNameLabel->setForegroundRole(DPalette::TextTitle);
    m_pAppNameLabel->setAccessibleName("DefSecurityToolsFrontUnit_AppNameLabel");
    m_pDesLabel = createLabel(12);
    m_pDesLabel->setForegroundRole(DPalette::TextTips);
    m_pDesLabel->setAccessibleName("DefSecurityToolsFrontUnit_DesLabel");
    m_pCompanyNameLabel = createLabel(10);
    m_pCompanyNameLabel->setForegroundRole(DPalette::PlaceholderText);
    m_pCompanyNameLabel->setAccessibleName("DefSecurityToolsFrontUnit_CompanyNameLabel");

    pVlayout->addWidget(m_pAppNameLabel);
    pVlayout->addWidget(m_pDesLabel);
    pVlayout->addWidget(m_pCompanyNameLabel);
    DPushButton *pBtn = creatorBtn(tr("Open", "button"), true);
    m_pOpenBtn = qobject_cast<DSuggestButton *>(pBtn);
    pBtn = creatorBtn(tr("Update", "button"), true);
    m_pUpdateBtn = qobject_cast<DSuggestButton *>(pBtn);
    m_pInstallBtn = creatorBtn(tr("Install", "button"), false);
    m_pUnInstallBtn = creatorBtn(tr("UnInstall", "button"), false);
    m_pSpinner = new DSpinner;
    m_pSpinner->setFixedSize(32, 32);
    m_pSpinner->hide();

    pMainLayout->addWidget(m_pImageLabel);
    pMainLayout->addLayout(pVlayout);
    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pInstallBtn);
    pMainLayout->addWidget(m_pUnInstallBtn);
    pMainLayout->addWidget(m_pOpenBtn);
    pMainLayout->addWidget(m_pUpdateBtn);
    pMainLayout->addWidget(m_pSpinner);
    setLayout(pMainLayout);
    setFixedSize(UNITWIDTH, UNITHEIGHT);
    setFrameRounded(true);
    setBackgroundRole(DPalette::ItemBackground);
}

void DefSecurityToolsFrontUnit::initConnection()
{
}

DLabel *DefSecurityToolsFrontUnit::createLabel(int iPixSize)
{
    DLabel *pLabel = new DLabel;
    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(iPixSize);
    pLabel->setFont(font);
    pLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    return pLabel;
}

DPushButton *DefSecurityToolsFrontUnit::creatorBtn(const QString &strText, bool bSuggest)
{
    DPushButton *pBtn = nullptr;

    if (bSuggest) {
        pBtn = new DSuggestButton;
    } else {
        pBtn = new DPushButton;
    }

    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);
    pBtn->setFont(font);
    //    QPalette pal = pBtn->palette();
    //    pal.setColor(QPalette::ButtonText, Qt::red);
    //    pBtn->setPalette(pal);
    pBtn->setFixedSize(60, 36);
    pBtn->setAccessibleName("DefSecurityToolsFrontUnit_" + strText);
    setShowString(pBtn, strText, 45);
    pBtn->hide();
    return pBtn;
}

void DefSecurityToolsFrontUnit::setShowString(DLabel *pLabel, const QString &strText, int iLen)
{
    QFont font = pLabel->font();
    QFontMetrics metrics(font);
    int iTextLen = metrics.horizontalAdvance(strText);
    QString strShowText = strText;

    if (iTextLen > iLen) {
        strShowText = metrics.elidedText(strText, Qt::ElideRight, iLen);
        pLabel->setToolTip(strText);
    }

    pLabel->setText(strShowText);
}

void DefSecurityToolsFrontUnit::setShowString(DPushButton *pBtn, const QString &strText, int iLen)
{
    QFont font = pBtn->font();
    QFontMetrics metrics(font);
    int iTextLen = metrics.horizontalAdvance(strText);
    QString strShowText = strText;

    if (iTextLen > iLen) {
        strShowText = metrics.elidedText(strText, Qt::ElideRight, iLen);
        pBtn->setToolTip(strText);
    }

    pBtn->setText(strShowText);
}

void DefSecurityToolsFrontUnit::initTranslation()
{
    transMap.clear();

    // 模块名称
    transMap.insert("Startup Programs", tr("Startup Programs"));
    transMap.insert("Network Detection", tr("Network Detection"));

    // 模块描述
    transMap.insert("Manage auto-startup programs", tr("Manage auto-startup programs"));
    transMap.insert("One-click detection, comprehensive network troubleshooting", tr("One-click detection, comprehensive network troubleshooting"));
}

QString DefSecurityToolsFrontUnit::getTranslation(const QString &strKey)
{
    QString strValue = strKey;

    if (transMap.contains(strKey)) {
        strValue = transMap.value(strKey);
    }

    return strValue;
}

void DefSecurityToolsFrontUnit::reportStatus(const QString &strMsg)
{
    DFloatingMessage *floMsgSwitchOff = nullptr;
    floMsgSwitchOff = new DFloatingMessage(DFloatingMessage::TransientType);
    floMsgSwitchOff->setIcon(QIcon::fromTheme(DIALOG_WARNING_TIP_RED));
    floMsgSwitchOff->setDuration(3000);
    floMsgSwitchOff->setMessage(strMsg);
    DMessageManager::instance()->sendMessage(this->parentWidget(), floMsgSwitchOff);
    DMessageManager::instance()->setContentMargens(this->parentWidget(), QMargins(0, 0, 0, 20));
}
