// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modeswitch.h"
#include "util.h"
#include "iconbutton.h"

#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QPainterPath>

ModeSwitch::ModeSwitch(QWidget *parent)
    : QWidget(parent)
    , m_titleModeButton(new IconButton(this))
    , m_letterModeButton(new IconButton(this))
    , m_buttonGrp(new QButtonGroup(this))
    , m_modeSettings(new QSettings(QSettings::UserScope, "deepin","dde-launcher-window-mode", this))
{
    initUi();
    initConnection();
    initAccessibleName();
}

ModeSwitch::~ModeSwitch()
{
}

void ModeSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

    // 8个像素的圆角以及相应的边框线和填充色
    QPainterPath path;
    QRect outsideRect = rect().marginsRemoved(QMargins(1, 1, 1, 1));
    path.addRoundedRect(outsideRect, 8, 8);
    QColor brushColor(Qt::white);
    brushColor.setAlpha(static_cast<int>(0.1 * 255));
    painter.fillPath(path, brushColor);
    painter.setBrush(QColor(200, 200, 200, static_cast<int>(0.3 * 255)));
    painter.drawRoundedRect(outsideRect, 8, 8, Qt::AbsoluteSize);

    return QWidget::paintEvent(event);
}

void ModeSwitch::onButtonClick(int id)
{
    m_modeSettings->setValue("mode", id);
}

void ModeSwitch::onButtonToggle(QAbstractButton *button, bool checked)
{
    IconButton *checkedButton = qobject_cast<IconButton *>(button);
    if (checkedButton)
        checkedButton->setChecked(checked);

    updateIcon();
}

void ModeSwitch::onThemeTypeChange(DGuiApplicationHelper::ColorType themeType)
{
    Q_UNUSED(themeType);

    // TODO: 等待对接设计师提供方案后，修改．
    //    if (themeType == DGuiApplicationHelper::DarkType)
    //        m_bgColor.setNamedColor("white");
    //    else
    //        m_bgColor.setNamedColor("black");

    //    m_bgColor.setAlpha(25);
    //    update();
}

void ModeSwitch::initUi()
{
    setWindowFlag(Qt::FramelessWindowHint);
    setFixedSize(57, 24);

    m_titleModeButton->setFixedSize(28, 24);
    m_titleModeButton->setIconSize({20, 20});
    m_letterModeButton->setFixedSize(28, 24);
    m_letterModeButton->setIconSize({20, 20});
    m_titleModeButton->setContentsMargins(6, 4, 6, 4);
    m_letterModeButton->setContentsMargins(6, 4, 6, 4);

    m_buttonGrp->addButton(m_titleModeButton, TitleMode);
    m_buttonGrp->addButton(m_letterModeButton, LetterMode);
    m_buttonGrp->setExclusive(true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    hLayout->addWidget(m_titleModeButton);
    hLayout->addWidget(m_letterModeButton);

    setLayout(hLayout);

    QPalette titleBtnPal = m_titleModeButton->palette();
    titleBtnPal.setBrush(QPalette::Highlight, Qt::transparent);
    titleBtnPal.setBrush(QPalette::Button, Qt::transparent);
    m_titleModeButton->setPalette(titleBtnPal);

    QPalette letterBtnPal = m_titleModeButton->palette();
    letterBtnPal.setBrush(QPalette::Highlight, Qt::transparent);
    letterBtnPal.setBrush(QPalette::Button, Qt::transparent);
    m_letterModeButton->setPalette(letterBtnPal);

    updateIcon();

    changeCategoryMode();
}

void ModeSwitch::initConnection()
{
    connect(m_buttonGrp, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ModeSwitch::onButtonClick);
    connect(m_buttonGrp, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ModeSwitch::buttonClicked);
    connect(m_buttonGrp, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, &ModeSwitch::onButtonToggle);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ModeSwitch::onThemeTypeChange);
}

void ModeSwitch::initAccessibleName()
{
    setAccessibleName("modeSwitchWidget");
    m_titleModeButton->setAccessibleName("categoryButton");
    m_letterModeButton->setAccessibleName("letterButton");
}

void ModeSwitch::updateIcon()
{
    if (m_letterModeButton->isChecked())
        m_letterModeButton->setIcon(QIcon(":/icons/skin/icons/letter-icon-checked.svg"));
    else
        m_letterModeButton->setIcon(QIcon(":/icons/skin/icons/letter-icon-normal.svg"));

    if (m_titleModeButton->isChecked())
        m_titleModeButton->setIcon(QIcon(":/icons/skin/icons/title-icon-checked.svg"));
    else
        m_titleModeButton->setIcon(QIcon(":/icons/skin/icons/title-icon-normal.svg"));

    update();
}

void ModeSwitch::changeCategoryMode()
{
    // 默认为字母模式
    int id = LetterMode;
    if (m_modeSettings->contains("mode"))
        id = m_modeSettings->value("mode").toInt();

    QMetaObject::invokeMethod(m_buttonGrp, "buttonClicked", Qt::QueuedConnection, Q_ARG(int, id));
    m_buttonGrp->button(id)->setChecked(true);
}
