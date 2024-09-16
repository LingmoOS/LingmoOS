// Copyright (C) 2016 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login.h"
#include "utils.h"
#include "trans_string.h"

#include <DSuggestButton>
#include <DPushButton>
#include <DFontSizeManager>
#include <DTipLabel>
#include <DSysInfo>
#include <DCommandLinkButton>

#include <QProcess>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE


QString systemName() {
    if (IsProfessionalSystem)
        return "UOS";

    return "Deepin";
}

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QVBoxLayout)
{
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    PicLabel *bgLabel = new PicLabel(this);
    bgLabel->SetImage(":/icons/deepin/builtin/icons/dcc_login_bg.svg");
    QVBoxLayout *logoLayout = new QVBoxLayout;
    bgLabel->setLayout(logoLayout);
    //logoLayout->setMargin(0);
    logoLayout->setSpacing(0);
    logoLayout->setContentsMargins(0, 0, 0, 0);
    DLabel *logoLabel = new DLabel;
    QImage logoimg(":/icons/deepin/builtin/icons/dcc_cloud_logo.svg");
    logoLabel->setScaledContents(true);
    logoLabel->setPixmap(QPixmap::fromImage(logoimg));
    logoLayout->addStretch(1);

    logoLayout->addSpacing(35);
    logoLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);

    //~ contents_path /cloudsync/Sign In
    DPushButton *loginBtn = new DPushButton(TransString::getTransString(STRING_LOGIN));
    loginBtn->setMinimumSize(170, 36);
    loginBtn->setFocusPolicy(Qt::NoFocus);
    DFontSizeManager::instance()->bind(loginBtn, DFontSizeManager::T7);

    DTipLabel *tip = new DTipLabel(TransString::getTransString(STRING_CLOUDTITLEMSG));
    tip->setWordWrap(true);
    tip->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(tip, DFontSizeManager::T9, QFont::Thin);

    m_mainLayout->addWidget(bgLabel, 1);

    QWidget *bottomWgt = new QWidget(this);
    QVBoxLayout *bottomLayout = new QVBoxLayout;

    QLabel *signintip = nullptr;
    signintip = new QLabel(TransString::getTransString(STRING_CLOUDSV));
    QFont font = signintip->font();
    font.setBold(true);
    signintip->setFont(font);
    DFontSizeManager::instance()->bind(signintip, DFontSizeManager::T2, QFont::DemiBold);

    bottomLayout->addWidget(signintip, 0, Qt::AlignHCenter);
    bottomLayout->addSpacing(6);
    bottomLayout->addWidget(tip);
    bottomLayout->addSpacing(35);
    bottomLayout->addWidget(loginBtn, 0, Qt::AlignHCenter);
    bottomLayout->addStretch();
    bottomWgt->setLayout(bottomLayout);
    m_mainLayout->addWidget(bottomWgt, 1);
    m_mainLayout->addSpacing(90);
    m_mainLayout->addStretch();

    setBackgroundRole(QPalette::Base);

    setLayout(m_mainLayout);
    connect(loginBtn, &QPushButton::clicked, this, &LoginPage::requestLoginUser);
}

QStringList LoginPage::getAgreementHttp()
{
    const QString& locale { QLocale::system().name() };
    QString useAgreement;
    QString privacyAgreement;
    QStringList agreement;
    if (locale == "zh_CN") {
        useAgreement = "https://www.uniontech.com/agreement/agreement-cn";
        privacyAgreement = "https://www.uniontech.com/agreement/id-privacy-cn";
    } else if (locale == "zh_HK") {
        useAgreement = "https://www.uniontech.com/agreement/agreement-hk";
        privacyAgreement = "https://www.uniontech.com/agreement/id-privacy-hk";
    } else if (locale == "zh_TW") {
        useAgreement = "https://www.uniontech.com/agreement/agreement-tw";
        privacyAgreement = "https://www.uniontech.com/agreement/id-privacy-tw";
    } else if (locale == "ug_CN") {
        useAgreement = "https://www.uniontech.com/agreement/agreement-uy";
        privacyAgreement = "https://www.uniontech.com/agreement/id-privacy-uy";
    } else if (locale == "bo_CN") {
        useAgreement = "https://www.uniontech.com/agreement/agreement-ti";
        privacyAgreement = "https://www.uniontech.com/agreement/id-privacy-ti";
    } else {
        useAgreement = "https://www.uniontech.com/agreement/agreement-en";
        privacyAgreement = "https://www.uniontech.com/agreement/id-privacy-en";
    }

    agreement.append(useAgreement);
    agreement.append(privacyAgreement);
    return agreement;
}

void LoginPage::onLinkActivated(const QString link)
{
    QDesktopServices::openUrl(QUrl(link));
}



PicLabel::PicLabel(QWidget *parent): QLabel (parent), m_svgrender(new QSvgRenderer(this))
{
    setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
}

void PicLabel::SetImage(const QString &imgurl)
{
    m_svgrender->load(imgurl);
    m_defSize = m_svgrender->defaultSize();
}

void PicLabel::resizeEvent(QResizeEvent *event)
{
    QSize finalSize = m_defSize;
    QSize newSize = size();
    if(m_defSize.width() < newSize.width() ||
        m_defSize.height() < newSize.height()) {
        finalSize.scale(newSize, Qt::KeepAspectRatioByExpanding);
    }

    QPixmap pixmap(finalSize * qApp->devicePixelRatio());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_svgrender->setAspectRatioMode(Qt::IgnoreAspectRatio);
    m_svgrender->render(&painter);
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    setPixmap(pixmap);
}
