// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applet.h"
#include "multitaskview.h"
#include "pluginfactory.h"
#include "../constants.h"

#include <QBuffer>

#include <DDciIcon>
#include <DDBusSender>
#include <DWindowManagerHelper>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

namespace dock {

constexpr int KWinOptimalPerformance = 4;
const QString windowEffectTypeKey = QStringLiteral("user_type");

MultiTaskView::MultiTaskView(QObject *parent)
    : DApplet(parent)
    , m_iconName("lingmo-multitasking-view")
{
    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, &MultiTaskView::visibleChanged);
    auto platformName = QGuiApplication::platformName();
    if (QStringLiteral("wayland") == platformName) {
        m_multitaskview.reset(new TreeLandMultitaskview);
    } else {
        m_kWinCompositingConfig = DConfig::create("org.kde.kwin", "org.kde.kwin.compositing", QString(), this);
        m_kWinEffect = m_kWinCompositingConfig->value(windowEffectTypeKey).toInt() != KWinOptimalPerformance;

        connect(m_kWinCompositingConfig, &DConfig::valueChanged, this, [=] (const QString &key) {
            if (key == windowEffectTypeKey) {
                bool kWinEffect = m_kWinCompositingConfig->value(windowEffectTypeKey).toInt() != KWinOptimalPerformance;
                if (kWinEffect != m_kWinEffect) {
                    m_kWinEffect = kWinEffect;
                    Q_EMIT visibleChanged();
                }
            }
        });
    }
}

bool MultiTaskView::init()
{
    DApplet::init();
    return true;
}

void MultiTaskView::openWorkspace()
{
    if (m_multitaskview) {
        m_multitaskview->toggle();
        return;
    }

    DDBusSender()
        .service("com.lingmo.wm")
        .path("/com/lingmo/wm")
        .interface("com.lingmo.wm")
        .method("ShowWorkspace")
        .call();
}

QString MultiTaskView::iconName() const
{
    return m_iconName;
}

void MultiTaskView::setIconName(const QString& iconName)
{
    if (iconName != m_iconName) {
        m_iconName = iconName;
        Q_EMIT iconNameChanged();
    }
}

bool MultiTaskView::visible() const
{
    return m_kWinEffect && m_visible && DWindowManagerHelper::instance()->hasComposite();
}

DockItemInfo MultiTaskView::dockItemInfo()
{
    DockItemInfo info;
    info.name = "multitasking-view";
    info.displayName = tr("Multitasking View");
    info.itemKey = "multitasking-view";
    info.settingKey = "multitasking-view";
    info.visible = visible();
    info.oceanuiIcon = DCCIconPath + "multitasking-view.svg";
    return info;
}

void MultiTaskView::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;

        Q_EMIT visibleChanged();
    }
}

D_APPLET_CLASS(MultiTaskView)
}


#include "multitaskview.moc"
