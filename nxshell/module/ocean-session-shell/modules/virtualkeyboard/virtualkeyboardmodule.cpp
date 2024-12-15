// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualkeyboardmodule.h"
#include "tipcontentwidget.h"
#include "keyboardiconwidget.h"
#include "virtualkbinstance.h"
#include "public_func.h"

#include <DConfig>

DCORE_USE_NAMESPACE

VirtualKeyboardModule::VirtualKeyboardModule(QObject *parent)
    : QObject(parent)
    , m_tipContentWidget(nullptr)
    , m_keyboardIconWidget(nullptr)
{
    setObjectName("VirtualKeyboardModule");
}

VirtualKeyboardModule::~VirtualKeyboardModule()
{
    if (m_tipContentWidget) {
        delete m_tipContentWidget;
    }

    if (m_keyboardIconWidget) {
        delete m_keyboardIconWidget;
    }
}

bool VirtualKeyboardModule::isNeedInitPlugin() const
{
    // wayland下不显示虚拟键盘（无法获取onboard界面的winId，导致异常）
    bool isInWayland = qgetenv("XDG_SESSION_TYPE").toLower().contains("wayland");

    // 根据配置，是否需要init插件
    DConfig *dConfig = DConfig::create(getDefaultConfigFileName(), getDefaultConfigFileName());
    dConfig->deleteLater();
    return (!isInWayland && dConfig && !dConfig->value("hideOnboard", false).toBool());
}

void VirtualKeyboardModule::init()
{
    m_tipContentWidget = new TipContentWidget();
    m_tipContentWidget->setText(tr("Onboard"));

    m_keyboardIconWidget = new KeyboardIconWidget();
    m_keyboardIconWidget->setIconPath(":/img/screen_keyboard_normal.svg");

    // 页面隐藏后，需要释放onboard进程
    connect(m_keyboardIconWidget, &KeyboardIconWidget::topLevelWidgetHided,
            &VirtualKBInstance::Instance(), &VirtualKBInstance::stopVirtualKBProcess);

    connect(m_keyboardIconWidget, &KeyboardIconWidget::iconWidgetHided,
            &VirtualKBInstance::Instance(), &VirtualKBInstance::hideKeyboardWidget);

    connect(m_keyboardIconWidget, &KeyboardIconWidget::clicked,
            &VirtualKBInstance::Instance(), &VirtualKBInstance::showKeyboardWidget);
}

QWidget *VirtualKeyboardModule::content()
{
    return nullptr;
}

QString VirtualKeyboardModule::icon() const
{
    return QString("");
}

QWidget *VirtualKeyboardModule::itemWidget() const
{
    return m_keyboardIconWidget;
}

QWidget *VirtualKeyboardModule::itemTipsWidget() const
{
    return m_tipContentWidget;
}

const QString VirtualKeyboardModule::itemContextMenu() const
{
    return QString("");
}

void VirtualKeyboardModule::invokedMenuItem(const QString &menuId, const bool checked) const
{
    Q_UNUSED(menuId)
    Q_UNUSED(checked)
}
