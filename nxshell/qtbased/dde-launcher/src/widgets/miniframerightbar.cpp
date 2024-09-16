// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "miniframerightbar.h"
#include "miniframebutton.h"
#include "util.h"

#include <DDesktopServices>
#include <DDBusSender>

#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QButtonGroup>
#include <QtConcurrent>

MiniFrameRightBar::MiniFrameRightBar(QWidget *parent)
    : QWidget(parent)
    , m_settingsBtn(new MiniFrameButton(tr("Settings"), this))
    , m_powerBtn(new MiniFrameButton(tr("Power"), this))
    , m_buttonGroup(new QButtonGroup(this))
    , m_hideList(SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "mini-frame-right-bar-hide-list", QStringList()).toStringList())
{
    initUi();
    initConnection();
    initAccessibleName();
}

MiniFrameRightBar::~MiniFrameRightBar()
{
}

bool MiniFrameRightBar::isButtonChecked(const ButtonType buttonId) const
{
    return m_buttonGroup->button(buttonId)->isChecked();
}

void MiniFrameRightBar::setButtonChecked(const ButtonType buttonId) const
{
    if (buttonId == MiniFrameRightBar::Setting) {
        m_settingsBtn->setChecked(true);
        m_powerBtn->setChecked(false);
    } else if (buttonId == MiniFrameRightBar::Power) {
        m_powerBtn->setChecked(true);
        m_settingsBtn->setChecked(false);
    } else {
        m_settingsBtn->setChecked(false);
        m_powerBtn->setChecked(false);
    }
}

void MiniFrameRightBar::initUi()
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_settingsBtn->setFixedHeight(36);
    m_powerBtn->setFixedHeight(36);
    m_settingsBtn->setIconSize(QSize(20, 20));
    m_powerBtn->setIconSize(QSize(20, 20));

    m_settingsBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_settingsBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonGroup->addButton(m_settingsBtn, MiniFrameRightBar::Setting);
    m_buttonGroup->addButton(m_powerBtn, MiniFrameRightBar::Power);
    m_settingsBtn->setCheckable(true);
    m_powerBtn->setCheckable(true);
    m_buttonGroup->setExclusive(false);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 6, 0, 6);
    hLayout->addStretch();
    hLayout->addWidget(m_powerBtn);
    hLayout->addStretch();
    hLayout->addWidget(m_settingsBtn);
    hLayout->addStretch();
    setLayout(hLayout);

    updateIcon(DGuiApplicationHelper::instance()->themeType());
}

void MiniFrameRightBar::initConnection()
{
    connect(m_settingsBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showSettings);
    connect(m_powerBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showShutdown);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MiniFrameRightBar::updateIcon);
}

void MiniFrameRightBar::initAccessibleName()
{
    setAccessibleName("buttonwidget");
    m_settingsBtn->setAccessibleName("settingsbtn");
    m_powerBtn->setAccessibleName("powerbtn");
}

void MiniFrameRightBar::showShutdown()
{
    DDBusSender()
            .service("org.deepin.dde.ShutdownFront1")
            .interface("org.deepin.dde.ShutdownFront1")
            .path("/org/deepin/dde/ShutdownFront1")
            .method("Show")
            .call();

    emit requestFrameHide();
}

void MiniFrameRightBar::showSettings()
{
    QtConcurrent::run([] {
        DDBusSender()
            .service("org.deepin.dde.ControlCenter1")
            .interface("org.deepin.dde.ControlCenter1")
            .path("/org/deepin/dde/ControlCenter1")
            .method(QString("Show"))
            .call();
    });

    emit requestFrameHide();
}

void MiniFrameRightBar::updateIcon(DGuiApplicationHelper::ColorType themeType)
{
    if (DGuiApplicationHelper::DarkType == themeType) {
         m_settingsBtn->setIcon(QIcon(":/widgets/images/setting_dark.svg"));
         m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown_dark.svg"));
     } else {
         m_settingsBtn->setIcon(QIcon(":/widgets/images/setting.svg"));
         m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown.svg"));
    }
}

void MiniFrameRightBar::changeEvent(QEvent *event)
{
#define ICON_WIDTH 20
#define GAP_WIDTH 20

    Q_UNUSED(event);

    QFontMetrics metrics(m_settingsBtn->font());
    QSize textSize = metrics.boundingRect(m_settingsBtn->text()).size();
    m_settingsBtn->setFixedSize(textSize + QSize(ICON_WIDTH, 10) + QSize(GAP_WIDTH, 0));

    textSize = metrics.boundingRect(m_settingsBtn->text()).size();
    m_powerBtn->setFixedSize(textSize + QSize(ICON_WIDTH, 10) + QSize(GAP_WIDTH, 0));
}
