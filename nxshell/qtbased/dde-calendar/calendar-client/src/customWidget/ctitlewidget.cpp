// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ctitlewidget.h"

#include "constants.h"
#include "configsettings.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QFocusEvent>

CTitleWidget::CTitleWidget(QWidget *parent)
    : QWidget(parent)
{
    m_sidebarIcon = new DIconButton(this);
    m_sidebarIcon->setFixedSize(QSize(36, 36));
    m_sidebarIcon->setIconSize(QSize(19, 15));
    connect(m_sidebarIcon, &DIconButton::clicked, this, &CTitleWidget::slotSidebarIconClicked);

    m_buttonBox = new CButtonBox(this);
    m_buttonBox->setObjectName("ButtonBox");
    m_buttonBox->setAccessibleName("ButtonBox");
    m_buttonBox->setAccessibleDescription("Year, month, week, day button box");

    DButtonBoxButton *m_yearButton = new DButtonBoxButton(tr("Y"), this);
    //设置年辅助技术显示名称
    m_yearButton->setObjectName("YearButton");
    m_yearButton->setAccessibleName("YearButton");
    m_yearButton->setFocusPolicy(Qt::TabFocus);
    m_yearButton->installEventFilter(this);

    QFont viewfont;
    viewfont.setWeight(QFont::Medium);
    viewfont.setPixelSize(DDECalendar::FontSizeFourteen);

    m_yearButton->setFixedSize(50, 36);
    DButtonBoxButton *m_monthButton = new DButtonBoxButton(tr("M"), this);
    //设置月辅助技术显示名称
    m_monthButton->setObjectName("MonthButton");
    m_monthButton->setAccessibleName("MonthButton");
    m_monthButton->setFixedSize(50, 36);
    m_monthButton->setFocusPolicy(Qt::TabFocus);
    m_monthButton->installEventFilter(this);

    DButtonBoxButton *m_weekButton = new DButtonBoxButton(tr("W"), this);
    //设置周辅助技术显示名称
    m_weekButton->setObjectName("WeekButton");
    m_weekButton->setAccessibleName("WeekButton");
    m_weekButton->setFixedSize(50, 36);
    m_weekButton->setFocusPolicy(Qt::TabFocus);
    m_weekButton->installEventFilter(this);

    DButtonBoxButton *m_dayButton = new DButtonBoxButton(tr("D"), this);
    //设置日辅助技术显示名称
    m_dayButton->setObjectName("DayButton");
    m_dayButton->setAccessibleName("DayButton");
    m_dayButton->setFixedSize(50, 36);
    m_dayButton->setFocusPolicy(Qt::TabFocus);
    m_dayButton->installEventFilter(this);

    m_yearButton->setFont(viewfont);
    m_monthButton->setFont(viewfont);
    m_weekButton->setFont(viewfont);
    m_dayButton->setFont(viewfont);

    QList<DButtonBoxButton *> btlist;
    btlist.append(m_yearButton);
    btlist.append(m_monthButton);
    btlist.append(m_weekButton);
    btlist.append(m_dayButton);
    m_buttonBox->setButtonList(btlist, true);

    m_buttonBox->setId(m_yearButton, DDECalendar::CalendarYearWindow);
    m_buttonBox->setId(m_monthButton, DDECalendar::CalendarMonthWindow);
    m_buttonBox->setId(m_weekButton, DDECalendar::CalendarWeekWindow);
    m_buttonBox->setId(m_dayButton, DDECalendar::CalendarDayWindow);
    m_buttonBox->setFixedSize(200, 36);

    m_searchEdit = new DSearchEdit(this);
    //设置搜索框辅助技术显示名称
    m_searchEdit->setObjectName("SearchEdit");
    m_searchEdit->setAccessibleName("SearchEdit");
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);
    m_searchEdit->setFixedHeight(36);
    m_searchEdit->setFont(viewfont);
    m_searchEdit->lineEdit()->installEventFilter(this);
    m_searchEdit->setPlaceHolder(tr("Search events and festivals"));
    m_searchEdit->setPlaceholderText(tr("Search events and festivals"));

    m_strPlaceHolder = m_searchEdit->placeholderText();
    connect(m_searchEdit, &DSearchEdit::searchAborted, [&] {
        //搜索框关闭按钮，清空数据
        slotSearchEditFocusChanged(false);
    });

    //搜索按钮，在窗口比较小的时候，显示搜索按钮隐藏搜索框
    m_searchPush = new DIconButton(this);
    m_searchPush->setFixedSize(36, 36);
    m_searchPush->setIcon(QIcon::fromTheme("search"));
    connect(m_searchPush, &DIconButton::clicked, this, &CTitleWidget::slotShowSearchEdit);

    //新建日程快捷按钮
    m_newScheduleBtn = new DIconButton(this);
    DStyle style;
    m_newScheduleBtn->setFixedSize(36, 36);
    //设置+
    m_newScheduleBtn->setIcon(style.standardIcon(DStyle::SP_IncreaseElement));

    //除新建日程按钮外的控件放在这个widget内，因为这些控件会根据窗口的大小显示变化而变化
    QWidget *leftWidget = new QWidget(this);
    {
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_sidebarIcon, Qt::AlignLeft);
        layout->addSpacing(4);
        layout->addWidget(m_buttonBox, Qt::AlignLeft);
        layout->addStretch();
        layout->addWidget(m_searchEdit, Qt::AlignCenter);
        layout->addStretch();
        layout->addWidget(m_searchPush, Qt::AlignRight);
        leftWidget->setLayout(layout);
    }

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(leftWidget);
    layout->addWidget(m_newScheduleBtn, Qt::AlignRight);
    this->setLayout(layout);
    //设置焦点代理为m_sidebarIcon
    setFocusProxy(m_sidebarIcon);
}

void CTitleWidget::setShowState(CTitleWidget::Title_State state)
{
    m_showState = state;
    stateUpdate();
}

void CTitleWidget::setSidebarStatus(bool status)
{
    m_sidebarstatus = status;
    updateSidebarIconStatus();
    m_clickShowLeft = status;
    emit signalSidebarStatusChange(m_sidebarstatus & m_sidebarCanDisplay);
    //将状态保存在配置文件中
    gSetting->setUserSidebarStatus(status);
}

void CTitleWidget::setSidebarCanDisplay(bool can)
{
    m_sidebarCanDisplay = can;
}

bool CTitleWidget::getSidevarStatus()
{
    return m_sidebarstatus;
}

DButtonBox *CTitleWidget::buttonBox() const
{
    return m_buttonBox;
}

DSearchEdit *CTitleWidget::searchEdit() const
{
    return m_searchEdit;
}

DIconButton *CTitleWidget::newScheduleBtn() const
{
    return m_newScheduleBtn;
}

void CTitleWidget::stateUpdate()
{
    switch (m_showState) {
    case Title_State_Mini: {
        //如果搜索框没有焦点且搜索框内没有内容则隐藏搜索框显示搜索图标按钮
        if (m_searchEdit->text().isEmpty() && !m_searchEdit->lineEdit()->hasFocus()) {
            m_searchPush->show();
            m_searchEdit->hide();
        } else {
            miniStateShowSearchEdit();
        }
    } break;
    default: {
        m_searchEdit->show();
        if (m_searchPush->hasFocus()) {
            m_searchEdit->setFocus();
        }
        m_buttonBox->show();
        m_searchPush->hide();
        normalStateUpdateSearchEditWidth();
    } break;
    }
}

void CTitleWidget::miniStateShowSearchEdit()
{
    m_buttonBox->hide();
    m_searchPush->hide();
    m_searchEdit->setMaximumWidth(width());
    m_searchEdit->show();

    m_searchEdit->setPlaceHolder(m_strPlaceHolder);
    m_searchEdit->setPlaceholderText(m_strPlaceHolder);
}

void CTitleWidget::normalStateUpdateSearchEditWidth()
{
    int padding = qMax(m_buttonBox->width(), m_newScheduleBtn->width());
    //更加widget宽度设置搜索框宽度
    int searchWidth = width() - 2 * padding;
    //参考其他应用设置宽度范围为 240～354
    if (searchWidth < 240) {
        searchWidth = 240;
    } else if (searchWidth > 354) {
        searchWidth = 354;
    }
    m_searchEdit->setMaximumWidth(searchWidth);
    m_searchEdit->setPlaceHolder(m_strPlaceHolder);
    m_searchEdit->setPlaceholderText(m_strPlaceHolder);
}

void CTitleWidget::updateSidebarIconStatus()
{
    m_sidebarIcon->setIcon(QIcon::fromTheme("dde_calendar_sidebar"));
}

void CTitleWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_showState == Title_State_Normal) {
        normalStateUpdateSearchEditWidth();
    }
    QString str  = m_strPlaceHolder;
    QFontMetrics fontMetrice(m_searchEdit->font());
    if (fontMetrice.width(str) > (m_searchEdit->width() - 30) && m_clickShowLeft == false && !m_buttonBox->isHidden()) {
        str = fontMetrice.elidedText(str, Qt::ElideRight, m_searchEdit->width() - 30);
        m_searchEdit->setPlaceHolder(str);
        m_searchEdit->setPlaceholderText(str);
    } else {
        m_searchEdit->setPlaceHolder(m_strPlaceHolder);
        m_searchEdit->setPlaceholderText(m_strPlaceHolder);
        m_clickShowLeft = false;
    }
}

void CTitleWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::FontChange) {
        updateSearchEditPlaceHolder();
    }
}

void  CTitleWidget::updateSearchEditPlaceHolder()
{
    QString str  = m_strPlaceHolder;
    QFontMetrics fontMetrice(m_searchEdit->font());
    if (fontMetrice.width(str) > (m_searchEdit->width() - 30)) {
        str = fontMetrice.elidedText(str, Qt::ElideRight, m_searchEdit->width() - 30);
        m_searchEdit->setPlaceHolder(str);
        m_searchEdit->setPlaceholderText(str);
    } else {
        m_searchEdit->setPlaceHolder(m_strPlaceHolder);
        m_searchEdit->setPlaceholderText(m_strPlaceHolder);
    }
}

bool CTitleWidget::eventFilter(QObject *o, QEvent *e)
{
    DButtonBoxButton *btn = qobject_cast<DButtonBoxButton *>(o);
    if (btn != nullptr && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(e);
        if (keyEvent != nullptr && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Space)) {
            emit signalSetButtonFocus();
        }
    }

    if (m_searchEdit != nullptr && m_searchEdit->lineEdit() == o) {
        if (e->type() == QEvent::FocusOut) {
            QFocusEvent *focusOutEvent = dynamic_cast<QFocusEvent *>(e);
            //如果为tab切换焦点则发送焦点切换信号
            if (focusOutEvent->reason() == Qt::TabFocusReason) {
                emit signalSearchFocusSwitch();
            }
            //根据焦点离开原因，决定是否隐藏搜索框
            if (focusOutEvent->reason() == Qt::TabFocusReason
                    || focusOutEvent->reason() == Qt::MouseFocusReason) {
                slotSearchEditFocusChanged(false);
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

void CTitleWidget::slotShowSearchEdit()
{
    miniStateShowSearchEdit();
    m_searchEdit->setFocus();

}

void CTitleWidget::slotSearchEditFocusChanged(bool onFocus)
{
    //如果获取焦点，或者搜索编辑框内容不为空则不处理
    if (onFocus || !m_searchEdit->text().isEmpty()) {
        return;
    }
    //如果为小窗口模式则隐藏部分控件
    if (m_showState == Title_State_Mini) {
        m_buttonBox->show();
        m_searchEdit->hide();
        m_searchPush->show();
    }
}

void CTitleWidget::slotSidebarIconClicked()
{
    //获取当前侧边栏显示状态
    bool display = m_sidebarCanDisplay & m_sidebarstatus;
    //点击按钮后侧边栏状态重新恢复为可显示状态
    m_sidebarCanDisplay = true;
    setSidebarStatus(!display); //切换显示状态
}
