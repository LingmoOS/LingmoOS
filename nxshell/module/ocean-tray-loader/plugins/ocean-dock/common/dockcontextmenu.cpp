// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dockcontextmenu.h"
#include "constants.h"
#include "../util/utils.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWindow>
#include <QPainter>
#include <QTimer>

DockContextMenu::DockContextMenu()
{
    // 解决键盘上下键不能操作右键菜单
    if (Utils::IS_WAYLAND_DISPLAY) {
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setProperty("_d_dwayland_window-type", "focusmenu");
    }
}

// action宽度的计算方法：文字宽度 + 小红点（如果有的话）+ 左右边距60
// 菜单的宽度为action的最大宽度与160（设计的最小宽度）两者取较大值
int DockContextMenu::suitableWidth() const
{
    int maxWidth = 0;
    for (auto action : actions()) {
        auto textWidth = fontMetrics().horizontalAdvance(action->text());
        textWidth += m_showReminderActions.contains(action) ? 26 : 0;
        maxWidth = maxWidth < textWidth ? textWidth : maxWidth;
    }

    return qMax(maxWidth + 60, 160);
}

void DockContextMenu::paintEvent(QPaintEvent* e)
{
    QMenu::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    for (auto action : m_showReminderActions) {
        auto geo = actionGeometry(action);
        QColor color("#FF3B30");
        p.setPen(color);
        p.setBrush(color);
        p.drawEllipse(geo.x() + geo.width() - 26, geo.y() + (geo.height() - 6) / 2, 6, 6);
    }

    p.end();
}

DockContextMenuHelper::DockContextMenuHelper()
    : QObject(nullptr)
    , m_currentWidget(nullptr)
{
    qApp->setProperty(MENU_IS_VISIBLE_OR_JUST_HIDE, false);
    connect(&m_contextMenu, &QMenu::triggered, this, [this] (QAction *action) {
        if (m_currentWidget) {
            Q_EMIT menuActionClicked(action, m_currentWidget.data());
        }
    });
}

QPoint DockContextMenuHelper::correctMenuPos(QPoint pos, int menuHeight)
{
    if (qApp->property(PROP_POSITION).value<Dock::Position>() != Dock::Position::Bottom)
        return pos;
    const QPoint &mousePos = QCursor::pos();
    const bool snapToMouse = QRect(pos.x() - 3, pos.y() - 3, 6, 6).contains(mousePos);
    if (!snapToMouse) {
        pos.setY(pos.y() - menuHeight);
    }
    return pos;
}

void DockContextMenuHelper::showContextMenu(QWidget* who, const QString& menuJson, const QPoint& pos)
{
    if (menuJson.isEmpty() || !who)
        return;

    m_currentWidget = who;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(menuJson.toLocal8Bit().data());
    if (jsonDocument.isNull())
        return;

    QJsonObject jsonMenu = jsonDocument.object();
    QJsonArray jsonMenuItems = jsonMenu.value("items").toArray();
    if (jsonMenuItems.size() == 0)
        return;

    m_contextMenu.clearShowReminderActions();
    qDeleteAll(m_contextMenu.actions());

    for (auto item : jsonMenuItems) {
        QJsonObject itemObj = item.toObject();
        auto* action = new QAction(itemObj.value("itemText").toString());
        action->setCheckable(itemObj.value("isCheckable").toBool());
        action->setChecked(itemObj.value("checked").toBool());
        action->setData(itemObj.value("itemId").toString());
        action->setEnabled(itemObj.value("isActive").toBool());
        if (itemObj.value("showReminder").toBool()) {
            m_contextMenu.addShowReminderAction(action);
        }
        m_contextMenu.addAction(action);
    }

    m_contextMenu.setFixedWidth(m_contextMenu.suitableWidth());

    if (!m_contextMenu.parentWidget())
        m_contextMenu.setParent(who->topLevelWidget(), Qt::Popup);

    qApp->setProperty(MENU_IS_VISIBLE_OR_JUST_HIDE, true);
    QPoint tpmPos = pos;
    tpmPos = correctMenuPos(tpmPos, m_contextMenu.sizeHint().height());
    m_contextMenu.exec(tpmPos);

    QTimer::singleShot(100, [] {
        qApp->setProperty(MENU_IS_VISIBLE_OR_JUST_HIDE, false);
    });
}

bool DockContextMenuHelper::menuIsVisible() const
{
    return m_contextMenu.isVisible();
}
