// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#if 0

#include "dmenucombobox.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QShowEvent>

DMenuComboBox::DMenuComboBox(DWidget *parent)
    : DWidget(parent)
    , m_currentIndex(-1)
{
    initUI();
    initConnection();
}

void DMenuComboBox::addItem(const QString &itemText)
{
    QIcon ico;
    addItem(itemText, ico);
}

void DMenuComboBox::addItem(QString itemText, QIcon icon)
{
    QAction *action = new QAction(this);
    action->setText(itemText);
    action->setIcon(icon);
    addItem(action);
}

void DMenuComboBox::addItem(QAction *action)
{
    action->setCheckable(true);
    m_menu->addAction(action);
    m_actions.append(action);

    if (m_currentIndex == -1) {
        m_currentIndex = 0;
        setCurrentIndex(m_currentIndex);
    }
}

void DMenuComboBox::removeItem(const QString &itemText)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (m_actions.at(i)->text() == itemText) {
            removeItem(i);
            break;
        }
    }
}

void DMenuComboBox::removeItem(int index)
{
    if (index >= m_actions.count() && index < 0) {
        qDebug() << "remove invalid index Item";
        return;
    }
    removeItem(m_actions.at(index));
}

void DMenuComboBox::removeItem(QAction *action)
{
    if (action == nullptr) {
        qDebug() << "remove invalid action Item with nullptr";
        return;
    }
    action->setParent(nullptr);
    m_menu->removeAction(action);
    m_actions.removeOne(action);

    if (m_currentIndex == m_actions.count()) {
        m_currentIndex--;
    }

    setCurrentIndex(m_currentIndex);
}

void DMenuComboBox::setCurrentIndex(int index)
{
    if (index >= m_actions.count() || index < 0 || m_actions.count() < 0) {
        qDebug() << "setCurrentIndex with invalid index...";
        return;
    }
    if (m_currentIndex < 0) {
        m_currentIndex = 0;
    } else if (m_currentIndex >= m_actions.count()) {
        m_currentIndex = m_actions.count() - 1;
    }

//    qDebug()<<"current Index:"<<index;

    m_actions.at(m_currentIndex)->setChecked(false);

    m_currentIndex = index;

    m_actions.at(m_currentIndex)->setChecked(true);

    // 更改按钮显示
    m_btn->setText(m_actions.at(m_currentIndex)->text());
    m_btn->setIcon(m_actions.at(m_currentIndex)->icon());

    emit signalCurrentTextChanged(m_actions.at(index)->text());
    emit signalCurrentIndexChanged(m_currentIndex);
}

//int DMenuComboBox::getCurrentIndex() const
//{
//    return m_currentIndex;
//}

void DMenuComboBox::setCurrentText(const QString &text)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (text == m_actions.at(i)->text()) {
            setCurrentIndex(i);
            break;
        }
    }
}

//QString DMenuComboBox::getCurrentText() const
//{
//    if (m_currentIndex >= m_actions.count() || m_currentIndex < 0 || m_actions.count() < 0) {
//        qDebug() << "setCurrentIndex with invalid index...";
//        return QString();
//    }
//    return m_actions.at(m_currentIndex)->text();
//}

void DMenuComboBox::setMenuFlat(bool flat)
{
    m_btn->setFlat(flat);
}

//void DMenuComboBox::setArrowDirction(Qt::LayoutDirection dir)
//{
//    m_btn->setLayoutDirection(dir);
//}

void DMenuComboBox::setItemICon(const QString &text, QIcon icon)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (text == m_actions.at(i)->text()) {
            setItemICon(i, icon);
        }
    }
}

void DMenuComboBox::setItemICon(int index, QIcon icon)
{
    if (index >= m_actions.count() || index < 0) {
        qDebug() << "setItemICon with invalid index...";
        return;
    }
    m_actions[index]->setIcon(icon);
}

//void DMenuComboBox::setMenuButtonICon(QString text, QIcon icon)
//{
//    m_btn->setText(text);
//    m_btn->setIcon(icon);
//}

//void DMenuComboBox::updateButtonTextAndIcon()
//{
//    if (m_currentIndex >= m_actions.count() || m_currentIndex < 0) {
//        qDebug() << "updateButtonTextAndIcon with invalid index:" << m_currentIndex;
//        return;
//    }
//    m_btn->setText(m_actions.at(m_currentIndex)->text());
//    m_btn->setIcon(m_actions.at(m_currentIndex)->icon());
//}

//void DMenuComboBox::setMenuMaxWidth(int width)
//{
//    m_menu->setMaximumWidth(width);
//}

void DMenuComboBox::slotActionToggled(QAction *action)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (action == m_actions.at(i)) {
            setCurrentIndex(i);
            emit signalActionToggled(action->text());
            break;
        }
    }
}

void DMenuComboBox::slotAboutToShow()
{
    emit signalAboutToShow();
}

void DMenuComboBox::initUI()
{
    m_btn = new DPushButton(this);
    m_menu = new QMenu(m_btn);

    QHBoxLayout *m_hlayout = new QHBoxLayout(this);
    m_hlayout->addWidget(m_btn);
    m_hlayout->setMargin(0);
    this->setLayout(m_hlayout);

    m_btn->setMenu(m_menu);
    m_menu->setMaximumWidth(110);
    m_btn->show();
}

void DMenuComboBox::initConnection()
{
    // 连接子选项按钮菜单被点击信号
    connect(m_menu, &QMenu::triggered, this, &DMenuComboBox::slotActionToggled);

    connect(m_menu, &QMenu::aboutToShow, this, &DMenuComboBox::slotAboutToShow);
}

#endif
