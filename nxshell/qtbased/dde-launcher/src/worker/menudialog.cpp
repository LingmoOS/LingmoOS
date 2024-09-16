// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menudialog.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include <QDBusConnection>

Menu::Menu(QWidget *parent)
    : QMenu(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::Dialog);
    setAccessibleName("popmenu");
    setObjectName("rightMenu");
    qApp->installEventFilter(this);

    // 点击任意区域均退出即可，启动器中菜单无二级菜单
    m_monitor = new XEventMonitor("org.deepin.dde.XEventMonitor1", "/org/deepin/dde/XEventMonitor1", QDBusConnection::sessionBus(), this);
    connect(m_monitor, &XEventMonitor::ButtonPress, this, &Menu::onButtonPress);
}

/** 右键菜单显示后在很多场景下都需要隐藏，为避免在各个控件中分别做隐藏右键菜单窗口的处理，
 *  因此这里统一做了处理。
 * @brief Menu::eventFilter
 * @param watched 过滤器监听对象
 * @param event 过滤器事件对象
 * @return 返回true, 事件不再向下传递，返回false，事件向下传递
 */
bool Menu::eventFilter(QObject *watched, QEvent *event)
{
    // 存在rightMenu和rightMenuWindow的对象名
    if (!watched->objectName().startsWith("rightMenu") && isVisible()) {
        if (event->type() == QEvent::DragMove || event->type() == QEvent::Wheel) {
            hide();
        }
        // 当右键菜单显示时捕获鼠标的release事件,click=press+release，
        // 让click无效，从而让启动器窗口不关闭
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->source() == Qt::MouseEventSynthesizedByQt)
                return true;
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

           if (keyEvent->key() == Qt::Key_Meta) {
               hide();
               return false;
           }

            int size = actions().size();
            if (size <= 0)
                return false;

            switch (keyEvent->key()) {
            case Qt::Key_Escape:
                hide();
                return true;
            case Qt::Key_Up:
                moveUp(size);
                return true;
            case Qt::Key_Down:
                moveDown();
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                // Qt::Key_Return[center enter], Qt::Key_Enter[right enter]
                openItem();
                return true;
            default:
                break;
            }
        }
    }

    return QMenu::eventFilter(watched, event);
}

void Menu::showEvent(QShowEvent *event)
{
    m_monitor->blockSignals(true);
    QMenu::showEvent(event);

    // dbus信号可能发来的稍微比点击操作慢一瞬间，会导致显示时立刻消失，做个很小的延时去避免这种现象
    QTimer::singleShot(10, this, [ = ] {
        m_monitor->blockSignals(false);
    });
}

void Menu::hideEvent(QHideEvent *event)
{
    qDebug() << Q_FUNC_INFO << "hide menu...";

    QMenu::hideEvent(event);

    m_monitor->blockSignals(true);
}

void Menu::moveDown(int size)
{
    Q_UNUSED(size);
    QAction *activeAction = this->activeAction();
    const int index = this->actions().indexOf(activeAction);

    for(int i = index + 1; i > -1; ++i) {
        // 循环一遍后找不到可用的action，直接退出
        if (i == index) {
            return;
        }

        // 从头查找
        if (i >= this->actions().size()) {
            i = 0;
        }

        auto act = this->actions().at(i);
        if (act->isSeparator() || !act->isEnabled()) {
            continue;
        }

        setActiveAction(act);
        break;
    }
}

void Menu::moveUp(int size)
{
    Q_UNUSED(size);
    QAction *activeAction = this->activeAction();
    const int index = this->actions().indexOf(activeAction);

    for(int i = index - 1; i < this->actions().size(); --i) {
        // 循环一遍后找不到可用的action，直接退出
        if (i == index) {
            return;
        }

        // 从末尾查找
        if (i <= -1) {
            i = this->actions().size() - 1;
        }

        auto act = this->actions().at(i);
        if (act->isSeparator() || !act->isEnabled()) {
            continue;
        }

        setActiveAction(act);
        break;
    }
}

void Menu::openItem()
{
    QAction *activeAction = this->activeAction();
    int index = this->actions().indexOf(activeAction);
    if (index != -1)
        activeAction->triggered();
}

void Menu::onButtonPress()
{
    if (!this->geometry().contains(QCursor::pos())) {
        this->hide();
    }
}
