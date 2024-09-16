// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "jobtypecombobox.h"

#include <DPushButton>

#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QPainterPath>
#include <QLineEdit>

JobTypeComboBox::JobTypeComboBox(QWidget *parent) : DComboBox(parent)
{
    initUI();
    //不启用自动匹配
    setAutoCompletion(false);
    //设置不接受回车键插入
    setInsertPolicy(QComboBox::NoInsert);
    this->view()->setTextElideMode(Qt::ElideRight);
    connect(this, QOverload<int>::of(&QComboBox::highlighted), [this](int index) {
        view()->setFocus();
        m_hoverSelectedIndex = index;
    });
}

JobTypeComboBox::~JobTypeComboBox()
{
    return;
}

QString JobTypeComboBox::getCurrentJobTypeNo()
{
    if (this->currentIndex() < 0 || this->currentIndex() >= m_lstJobType.size()) {
        return QString();
    }
    return m_lstJobType[this->currentIndex()]->typeID();
}

void JobTypeComboBox::setCurrentJobTypeNo(const QString &strJobTypeNo)
{
    for (int i = 0; i < m_lstJobType.size(); i++) {
        if (strJobTypeNo == m_lstJobType[i]->typeID()) {
            this->setCurrentIndex(i);
            break;
        }
    }
    return;
}

void JobTypeComboBox::setAlert(bool isAlert)
{
    if (m_control) {
        //输入框未显示警告色？
        m_control->setAlert(isAlert);
    }
}

bool JobTypeComboBox::isAlert() const
{
    if (m_control) {
        return m_control->isAlert();
    }
    return false;
}

void JobTypeComboBox::showAlertMessage(const QString &text, int duration)
{
    showAlertMessage(text, nullptr, duration);
}

void JobTypeComboBox::showAlertMessage(const QString &text, QWidget *follower, int duration)
{
    if (m_control) {
        m_control->showAlertMessage(text, follower ? follower : this, duration);
    }
}

void JobTypeComboBox::setAlertMessageAlignment(Qt::Alignment alignment)
{
    if (m_control) {
        m_control->setMessageAlignment(alignment);
    }
}

void JobTypeComboBox::hideAlertMessage()
{
    if (m_control) {
        m_control->hideAlertMessage();
    }
}

int JobTypeComboBox::getCurrentEditPosition() const
{
    return m_newPos;
}

void JobTypeComboBox::updateJobType(const AccountItem::Ptr& account)
{
    if (nullptr == account) {
        return;
    }

    //将自定义控件内存是否并置空，保存初始展示时能够进入高度调整
    if (m_customWidget) {
        m_addBtn->deleteLater();
        m_customWidget->deleteLater();
        m_addBtn = nullptr;
        m_customWidget = nullptr;
    }

    //切换账号重置选项后需重置view的高度限制和大小策略，使其在初次显示时能跟随列表项数量动态调整高度
    QFrame *viewContainer = findChild<QFrame *>();
    if (viewContainer) {
        viewContainer->setMinimumHeight(0);
        viewContainer->setMaximumHeight(16777215);
        viewContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }

    //保存现场
    bool isEnit = isEditable();
    QString text = currentText();
    m_lstJobType = account->getScheduleTypeList();
    clear(); //更新前先清空原有列表
    for (m_itemNumIndex = 0; m_itemNumIndex < m_lstJobType.size(); m_itemNumIndex++) {
        addJobTypeItem(m_itemNumIndex, m_lstJobType[m_itemNumIndex]->getColorCode(), m_lstJobType[m_itemNumIndex]->displayName());
    }
    //数据重置后hover标识重置为-1
    m_hoverSelectedIndex = -1;
    //恢复原状
    setEditable(isEnit);
    setCurrentText(text);

}

void JobTypeComboBox::addJobTypeItem(int idx, QString strColorHex, QString strJobType)
{
    //绘制的pixmap为基准大小的4倍，防止缩放时出现齿距
    QSize size(64, 64);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QColor(strColorHex));
    painter.setPen(QColor(strColorHex));
    painter.drawRoundedRect(0, 0, 64, 64, 16, 16);

    //描边
    QPainterPath path;
    path.addRoundedRect(0, 0, 64, 64, 16, 16);
    path.addRoundedRect(4, 4, 56, 56, 16, 16);
    painter.setBrush(QColor(0, 0, 0, 255 / 10));
    painter.drawPath(path);

    insertItem(idx, QIcon(pixmap), tr(strJobType.toLocal8Bit()));
}

void JobTypeComboBox::initUI()
{
    setEditable(false);
    setIconSize(QSize(16, 16));
}

void JobTypeComboBox::setItemSelectable(bool status)
{
    if (-1 == m_hoverSelectedIndex) {
        return;
    }
    //更改当前列表框里的高亮项的可选择状态设置，将其设置为不可选中状态可实现失去选中效果，待聚焦后再恢复过来
    QStandardItemModel *pItemModel = qobject_cast<QStandardItemModel *>(model());
    pItemModel->item(m_hoverSelectedIndex)->setSelectable(status);

    //设置“添加按键”按钮的高亮状态
    if (nullptr != m_addBtn) {
        m_addBtn->setHighlight(!status);
    }
}

void JobTypeComboBox::addCustomWidget(QFrame *viewContainer)
{
    if (viewContainer) {
        if (nullptr == m_customWidget) {
            //获取原控件布局
            QBoxLayout *layout = qobject_cast<QBoxLayout *>(viewContainer->layout());
            //自定义控件
            QVBoxLayout *hLayout = new QVBoxLayout;
            DPushButton *splitter = new DPushButton(this);
            m_addBtn = new CPushButton();
            splitter->setFixedHeight(2);
            splitter->setFocusPolicy(Qt::NoFocus);
            hLayout->addWidget(splitter);
            hLayout->addWidget(m_addBtn);
            hLayout->setContentsMargins(0, 0, 0, 0);
            hLayout->setSpacing(0);
            m_customWidget = new QWidget(this);
            m_customWidget->setFixedHeight(35);
            m_customWidget->setLayout(hLayout);
            //添加自定义控件到最后
            layout->insertWidget(-1, m_customWidget);
            viewContainer->setFixedHeight(viewContainer->height() + m_customWidget->height());
            //设置可接受tab焦点
            m_addBtn->setFocusPolicy(Qt::TabFocus);
            setTabOrder(view(), m_addBtn);
            //注册事件监听
            m_addBtn->installEventFilter(this);
            view()->installEventFilter(this);
            connect(m_addBtn, &CPushButton::clicked, this, &JobTypeComboBox::slotBtnAddItemClicked);
        }
    }
}

void JobTypeComboBox::showPopup()
{
    //重置icon大小
    setIconSize(QSize(16, 16));
    setItemSelectable(true);
    if (currentIndex() < 0)
        setCurrentIndex(0);
    //设置为不可编辑模式
    setEditable(false);
    //下拉模式取消信号关联，并释放警报控制
    if (m_control) {
        disconnect(m_control, &DAlertControl::alertChanged, this, &JobTypeComboBox::alertChanged);
        delete m_control;
        m_control = nullptr;
    }

    emit activated(0);
    DComboBox::showPopup();

    //获取下拉视图容器
    QFrame *viewContainer = findChild<QFrame *>();

    if (nullptr == m_customWidget) {
        //添加自定义布局
        addCustomWidget(viewContainer);
    }

    if (m_customWidget && m_lstJobType.size() >= 20) {
        m_customWidget->hide();
    }

    //设置最大高度为400
    if (viewContainer->height() > 400) {
        viewContainer->setFixedHeight(400);
    }
    //重新调整选中项的位置
    view()->scrollTo(view()->currentIndex(), QAbstractItemView::PositionAtCenter);

}

bool JobTypeComboBox::eventFilter(QObject *obj, QEvent *event)
{
    if (view() == obj && (event->type() == QEvent::Enter)) {
        view()->setFocus();
    } else if (m_addBtn == obj && (event->type() == QEvent::Enter)) {
        m_addBtn->setFocus();
    } else if (view() == obj && event->type() == QEvent::FocusIn) {
        //列表框控件焦点进入事件
        setItemSelectable(true);
    } else if (m_addBtn == obj && event->type() == QEvent::FocusIn) {
        //“添加按键”控件焦点进入事件
        setItemSelectable(false);
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEvent = dynamic_cast<QKeyEvent *>(event);
        if (view() == obj && kEvent->key() == Qt::Key_Down) {
            //焦点在列表框时的下方向按键按下事件
            if (m_addBtn->isHighlight()) {
                return true;
            }

            if (m_hoverSelectedIndex == m_itemNumIndex - 1) {
                //列表框到达最后一项时的下方向按键按下事件
                //将焦点转移到“添加按键”控件上
                m_addBtn->setFocus();

            }
        } else if (m_addBtn == obj && kEvent->key() == Qt::Key_Up) {
            //焦点在m_addBtn时的上方向按键按下事件
            if (m_addBtn->isHighlight()) {
                //将焦点转移到列表控件上
                view()->setFocus();
            }
        } else if (m_addBtn->isHighlight() && kEvent->key() == Qt::Key_Return) {
            //回车事件
            slotBtnAddItemClicked();
        }

        //过滤掉m_addBtn的所有按键事件
        if (m_addBtn == obj) {
            return true;
        }
    }
    return DComboBox::eventFilter(obj, event);
}

void JobTypeComboBox::slotBtnAddItemClicked()
{
    JobTypeComboBox::hidePopup();
    setIconSize(QSize(0, 0));
    //设置没有选中，
    //    setCurrentIndex(-1);
    setEditable(true);
    setCurrentText("");
    //设置为编辑模式后才会创建lineEdit
    m_control = new DAlertControl(this->lineEdit(), this);
    connect(m_control, &DAlertControl::alertChanged, this, &JobTypeComboBox::alertChanged);
    connect(lineEdit(), &QLineEdit::editingFinished, this, &JobTypeComboBox::slotEditingFinished);
    connect(lineEdit(), &QLineEdit::cursorPositionChanged, this, &JobTypeComboBox::slotEditCursorPositionChanged);
    emit signalAddTypeBtnClicked();
    return;
}

void JobTypeComboBox::slotEditingFinished()
{
    //TODO:待优化，由于上下按键会匹配下拉列表内容，导致会调整lineEdit显示位置
    //当前先改回设置显示图标的方法
    //    int oldPos = m_newPos;
    //    QString str = lineEdit()->text();
    //    setCurrentIndex(-1);
    //    setCurrentText(str);
    //    lineEdit()->setCursorPosition(oldPos);
    emit editingFinished();
}

void JobTypeComboBox::slotEditCursorPositionChanged(int oldPos, int newPos)
{
    m_oldPos = oldPos;
    m_newPos = newPos;
}
