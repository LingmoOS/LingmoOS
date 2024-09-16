// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "passwordlevelwidget.h"

#include <QHBoxLayout>

#include <DLabel>


#define IMG_PREFIX (QString(":/src/widgets/img/password_level/"))
PasswordLevelWidget::PasswordLevelWidget(QWidget *parent)
    : QWidget(parent)
    , m_level(PASSWORD_STRENGTH_LEVEL_ERROR)
    , m_layout(new QHBoxLayout(this))
    , m_tips(new DLabel(tr(""), this))
    , m_lowIcon(new DLabel(this))
    , m_mediumIcon(new DLabel(this))
    , m_highIcon(new DLabel(this))
{
    initUI();
    initConnections();
}

void PasswordLevelWidget::reset()
{
    m_level = PASSWORD_STRENGTH_LEVEL_ERROR;

    m_tips->clear();
    m_lowIcon->setPixmap(defaultIcon());
    m_mediumIcon->setPixmap(defaultIcon());
    m_highIcon->setPixmap(defaultIcon());
}

void PasswordLevelWidget::setLevel(PASSWORD_LEVEL_TYPE level)
{
    if (m_level == level)
        return;

    m_level = level;

    QPalette palette = m_tips->palette();
    switch (level){
    case PASSWORD_STRENGTH_LEVEL_LOW:
    {
        palette.setColor(QPalette::WindowText, QColor("#FF5736"));
        m_tips->setPalette(palette);
        m_tips->setText(tr("Weak"));

        m_lowIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_low.svg"));
        m_mediumIcon->setPixmap(defaultIcon());
        m_highIcon->setPixmap(defaultIcon());
    }
        break;
    case PASSWORD_STRENGTH_LEVEL_MIDDLE:
    {
        palette.setColor(QPalette::WindowText, QColor("#FFAA00"));
        m_tips->setPalette(palette);
        m_tips->setText(tr("Medium"));

        m_lowIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_middle.svg"));
        m_mediumIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_middle.svg"));
        m_highIcon->setPixmap(defaultIcon());
    }
        break;
    case PASSWORD_STRENGTH_LEVEL_HIGH:
    {
        palette.setColor(QPalette::WindowText, QColor("#15BB18"));
        m_tips->setPalette(palette);
        m_tips->setText(tr("Strong"));
        m_lowIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_high.svg"));
        m_mediumIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_high.svg"));
        m_highIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_high.svg"));
    }
        break;
    default:
        reset();
        break;
    }
}

void PasswordLevelWidget::initUI()
{
    m_tips->clear();
    m_lowIcon->setPixmap(defaultIcon());
    m_mediumIcon->setPixmap(defaultIcon());
    m_highIcon->setPixmap(defaultIcon());

    m_layout->addWidget(m_tips);
    m_layout->addWidget(m_lowIcon);
    m_layout->addWidget(m_mediumIcon);
    m_layout->addWidget(m_highIcon);
    m_layout->setMargin(0);
    setLayout(m_layout);
}

void PasswordLevelWidget::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &PasswordLevelWidget::onThemeTypeChanged);
}

QString PasswordLevelWidget::defaultIcon()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        return IMG_PREFIX + "dcc_deepin_password_strength_unactive_light_mode.svg";
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        return IMG_PREFIX + "dcc_deepin_password_strength_unactive_deep_mode.svg";
    } else {
        return QString();
    }
}

void PasswordLevelWidget::onThemeTypeChanged(DGuiApplicationHelper::ColorType type)
{
    Q_UNUSED(type)

    switch (m_level) {
    case PASSWORD_STRENGTH_LEVEL_LOW:
    {
        m_lowIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_low.svg"));
        m_mediumIcon->setPixmap(defaultIcon());
        m_highIcon->setPixmap(defaultIcon());
    }
        break;
    case PASSWORD_STRENGTH_LEVEL_MIDDLE:
    {
        m_lowIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_middle.svg"));
        m_mediumIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_middle.svg"));
        m_highIcon->setPixmap(defaultIcon());
    }
        break;
    case PASSWORD_STRENGTH_LEVEL_HIGH:
    {
        m_lowIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_high.svg"));
        m_mediumIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_high.svg"));
        m_highIcon->setPixmap(QPixmap(IMG_PREFIX + "dcc_deepin_password_strength_high.svg"));
    }
        break;
    default:
        reset();
        break;
    }
}
