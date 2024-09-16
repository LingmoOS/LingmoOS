// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dzoommenucombobox.h"
#include "application.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QPalette>
#include <QStyle>
#include <QRect>

#include <DPushButton>
#include <DIconButton>
#include <QKeyEvent>
#include <QComboBox>

DZoomMenuComboBox::DZoomMenuComboBox(DWidget *parent):
    DWidget(parent)
    , m_floatingSize(32)
    , m_currentIndex(-1)
{
    initUI();
    initConnection();
}

DZoomMenuComboBox::~DZoomMenuComboBox()
{
    if (_btnLay != nullptr) {
        _btnLay->deleteLater();
        _btnLay = nullptr;
    }
}

void DZoomMenuComboBox::addItem(const QString &itemText)
{
    QIcon ico;
    addItem(itemText, ico);
}

void DZoomMenuComboBox::addItem(QString itemText, QIcon icon)
{
    QAction *action = new QAction(this);
    action->setText(itemText);
    action->setIcon(icon);
    addItem(action);
}

void DZoomMenuComboBox::addItem(QAction *action)
{
    action->setCheckable(false);
    action->setShortcuts(QKeySequence::UnknownKey);
    action->setAutoRepeat(false);
    m_menu->addAction(action);
    m_actions.append(action);

    if (m_currentIndex == -1) {
        m_currentIndex = 0;
        setCurrentIndex(m_currentIndex);
    }
}

void DZoomMenuComboBox::removeItem(const QString &itemText)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (m_actions.at(i)->text() == itemText) {
            removeItem(i);
            break;
        }
    }
}

void DZoomMenuComboBox::removeItem(int index)
{
    if (index >= m_actions.count() || index < 0) {
        qDebug() << "remove invalid index Item";
        return;
    }
    removeItem(m_actions.at(index));
}

void DZoomMenuComboBox::removeItem(QAction *action)
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

void DZoomMenuComboBox::setCurrentIndex(int index)
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

    m_actions.at(m_currentIndex)->setChecked(false);

    m_currentIndex = index;

    m_actions.at(m_currentIndex)->setChecked(true);
    // 更改按钮显示
    setMenuButtonTextAndIcon(m_actions.at(m_currentIndex)->text(), m_actions.at(m_currentIndex)->icon());
    emit signalCurrentTextChanged(m_actions.at(index)->text());
    emit signalCurrentIndexChanged(m_currentIndex);
}

void DZoomMenuComboBox::setCurrentText(const QString &text)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (text == m_actions.at(i)->text()) {
            setCurrentIndex(i);
            break;
        }
    }
}

void DZoomMenuComboBox::setItemICon(const QString &text, const QIcon icon)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (text == m_actions.at(i)->text()) {
            setItemICon(i, icon);
        }
    }
}

void DZoomMenuComboBox::setItemICon(int index, QIcon icon)
{
    if (index >= m_actions.count() || index < 0) {
        qDebug() << "setItemICon with invalid index...";
        return;
    }
    m_actions[index]->setIcon(icon);
}

void DZoomMenuComboBox::setMenuButtonTextAndIcon(QString text, QIcon ico)
{
    m_label->setText(text);
}

void DZoomMenuComboBox::slotActionToggled(QAction *action)
{
    for (int i = 0; i < m_actions.count(); i++) {
        if (action->text() == m_actions.at(i)->text()) {
            setCurrentIndex(i);
            break;
        }
    }
}

bool DZoomMenuComboBox::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_menu) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *pKeyEvent = dynamic_cast<QKeyEvent *>(e);

            if (!pKeyEvent->modifiers() && pKeyEvent->text().length() == 1) {
                return true;
            }
        }
    } else if (o == m_label) {
        // 比例文本被点击时触发显示下拉列表
        if (e->type() == QEvent::MouseButtonPress) {
            m_menu->exec(mapToGlobal(QPoint(0, this->geometry().y() + this->geometry().height())));
        }
    }
    return DWidget::eventFilter(o, e);
}

void DZoomMenuComboBox::initUI()
{
    setWgtAccesibleName(this, "Zoom Form");
    // [0] 实例化菜单按钮
    m_btn = new QPushButton("", this);
    setWgtAccesibleName(m_btn, "Zoom Menu button");
    m_menu = new QMenu(this);
    setWgtAccesibleName(m_menu, "Zoom Menu");
    m_menu->installEventFilter(this);
    m_btn->setObjectName("ScanleBtn");
    m_btn->setFlat(true);
    connect(m_btn, &QPushButton::clicked, this, [ = ]() {
        m_menu->exec(mapToGlobal(QPoint(0, this->geometry().y() + this->geometry().height())));
    });

    // [1] 左右加减按钮
    m_increaseBtn = new DIconButton(this);
    m_reduceBtn = new DIconButton(this);
    m_increaseBtn->setFlat(true);
    m_reduceBtn->setFlat(true);
    //显示
    m_label = new QLabel(this);
    m_label->installEventFilter(this);

    setWgtAccesibleName(m_increaseBtn, "Zoom increase button");
    setWgtAccesibleName(m_reduceBtn,   "Zoom reduce button");

    m_increaseBtn->setIcon(QIcon::fromTheme("ddc_button_add_hover"));
    m_reduceBtn->setIcon(QIcon::fromTheme("ddc_button_reduce_hover"));
    m_btn->setIcon(QIcon::fromTheme("ddc_drop_down"));

    m_increaseBtn->setFixedSize(QSize(m_floatingSize, m_floatingSize));
    m_reduceBtn->setFixedSize(QSize(m_floatingSize, m_floatingSize));
    m_reduceBtn->setBackgroundRole(QPalette::Button);
    m_increaseBtn->setBackgroundRole(QPalette::Button);
    m_reduceBtn->setIconSize(QSize(16, 16));
    m_increaseBtn->setIconSize(QSize(16, 16));
    m_btn->setIconSize(QSize(24, 24));
    m_label->setFixedSize(QSize(50, 24));

    connect(m_reduceBtn, &DFloatingButton::clicked, this, [ = ]() {
        emit signalLeftBtnClicked();
    });
    connect(m_increaseBtn, &DFloatingButton::clicked, this, [ = ]() {
        emit signalRightBtnClicked();
    });

    _btnLay = new QHBoxLayout();
    _btnLay->addWidget(m_reduceBtn);
    _btnLay->addWidget(m_label);
    _btnLay->addWidget(m_btn);
    _btnLay->addWidget(m_increaseBtn);
    setLayout(_btnLay);
}

void DZoomMenuComboBox::initConnection()
{
    // 连接子选项按钮菜单被点击信号
    connect(m_menu, &QMenu::triggered, this, &DZoomMenuComboBox::slotActionToggled);
    connect(m_menu, &QMenu::aboutToHide, this, [ = ]() {
        this->setFocus();
    });
}
