// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "myscheduleview.h"
#include "scheduledlg.h"
#include "scheduledatamanage.h"
#include "cdynamicicon.h"
#include "constants.h"
#include "cscheduleoperation.h"
#include "lunarmanager.h"


#include <DPalette>
#include <DFontSizeManager>
#include <DLabel>

#include <QShortcut>
#include <QVBoxLayout>
#include <QPainter>
#include <QtMath>

DGUI_USE_NAMESPACE
CMyScheduleView::CMyScheduleView(const DSchedule::Ptr &schduleInfo, QWidget *parent)
    : DCalendarDDialog(parent)
{
    setContentsMargins(0, 0, 0, 0);
    m_scheduleInfo = schduleInfo;
    initUI();
    initConnection();
    //根据主题type设置颜色
    setLabelTextColor(DGuiApplicationHelper::instance()->themeType());
    setFixedSize(400, 160);
    //设置初始化弹窗内容
    updateDateTimeFormat();
    focusNextPrevChild(false);
    slotAccountStateChange();
}

void CMyScheduleView::setSchedules(const DSchedule::Ptr &schduleInfo)
{
    m_scheduleInfo = schduleInfo;
}

void CMyScheduleView::updateFormat()
{
    updateDateTimeFormat();
    slotAccountStateChange();
}

/**
 * @brief CMyScheduleView::AutoFeed     字体改变更改界面显示
 * @param text
 */
void CMyScheduleView::slotAutoFeed(const QFont &font)
{
    Q_UNUSED(font)
    if (nullptr == m_timeLabel || nullptr == m_scheduleLabel) {
        return;
    }
    QString strText = m_scheduleInfo->summary();
    QString resultStr = nullptr;
    QFont labelF;
    labelF.setWeight(QFont::Medium);
    labelF = DFontSizeManager::instance()->get(DFontSizeManager::T6, labelF);
    QFontMetrics fm(labelF);
    int titlewidth = fm.horizontalAdvance(strText);
    QStringList strList;
    QString str;
    int h = fm.height();
    strList.clear();

    if (titlewidth < 330) {
        strList.append(strText);
        resultStr += strText;
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);

            if (fm.horizontalAdvance(str) > 330) {
                str.remove(str.count() - 1, 1);
                strList.append(str);
                resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        strList.append(str);
        resultStr += str;
    }

    if (strList.count() * h > 100) {
        m_scheduleLabelH = 100;
    } else {
        int minH = 17;
        m_scheduleLabelH = strList.count() * h;
        m_scheduleLabelH = m_scheduleLabelH >= minH ? m_scheduleLabelH : minH;
    }
    //更新控件高度
    area->setFixedHeight(m_scheduleLabelH);
    m_scheduleLabel->setText(resultStr);

    m_timeLabelH = 26;
    if (m_scheduleInfo->lunnar()) {
        QString timeName = m_timeLabel->text();
        int index = timeName.indexOf("~");
        //重新计算法字符串像素长度
        if (index != -1) {
            timeName[index - 1] = ' ';
            QFontMetrics fm(m_timeLabel->font());
            int textWidth = fm.horizontalAdvance(m_timeLabel->text());
            if (textWidth > m_timeLabel->width()) {
                timeName[index - 1] = '\n';
                m_timeLabelH = 58;
            } else {
                m_timeLabelH = 26;
            }
        }
        m_timeLabel->setText(timeName);
    } else {
        m_timeLabelH = 26;
    }
    //更新控件高度
    m_timeLabel->setFixedHeight(m_timeLabelH);

    //更新界面高度
    setFixedHeight(m_defaultH + m_timeLabelH + m_scheduleLabelH);
}

void CMyScheduleView::slotAccountStateChange()
{
    AccountItem::Ptr item = gAccountManager->getAccountItemByScheduleTypeId(m_scheduleInfo->scheduleTypeID());
    if (!item) {
        return;
    }
    //根据可同步状态设置删除按钮是否可用
    getButtons()[0]->setEnabled(item->isCanSyncShedule());
}

/**
 * @brief setLabelTextColor     设置label文字颜色
 * @param type  主题type
 */
void CMyScheduleView::setLabelTextColor(const int type)
{
    //标题显示颜色
    QColor titleColor;
    //日程显示颜色
    QColor scheduleTitleColor;
    //时间显示颜色
    QColor timeColor;
    if (type == 2) {
        titleColor = "#C0C6D4";
        scheduleTitleColor = "#FFFFFF";
        timeColor = "#FFFFFF";
        timeColor.setAlphaF(0.7);
    } else {
        titleColor = "#001A2E";
        scheduleTitleColor = "#000000";
        scheduleTitleColor.setAlphaF(0.9);
        timeColor = "#000000";
        timeColor.setAlphaF(0.6);
    }
    //设置颜色
    setPaletteTextColor(m_Title, titleColor);
    setPaletteTextColor(m_scheduleLabel, scheduleTitleColor);
    setPaletteTextColor(m_timeLabel, timeColor);
}

/**
 * @brief setPaletteTextColor   设置调色板颜色
 * @param widget                需要设置的widget
 * @param textColor             显示颜色
 */
void CMyScheduleView::setPaletteTextColor(QWidget *widget, QColor textColor)
{
    //如果为空指针则退出
    if (nullptr == widget)
        return;
    DPalette palette = widget->palette();
    //设置文字显示颜色
    palette.setColor(DPalette::WindowText, textColor);
    widget->setPalette(palette);
}

/**
 * @brief CMyScheduleView::updateDateTimeFormat 更新显示时间格式
 */
void CMyScheduleView::updateDateTimeFormat()
{
    //如果为节假日
    if (CScheduleOperation::isFestival(m_scheduleInfo)) {
        m_timeLabel->setText(m_scheduleInfo->dtStart().toString(m_dateFormat));
    } else {
        QString showTime;
        QString beginName, endName;
        if (m_scheduleInfo->allDay()) {
            if (m_scheduleInfo->isMultiDay()) {
                beginName = getDataByFormat(m_scheduleInfo->dtStart().date(), m_dateFormat);
                endName = getDataByFormat(m_scheduleInfo->dtEnd().date(), m_dateFormat);
                showTime = beginName + " ~ " + endName;
            } else {
                showTime = getDataByFormat(m_scheduleInfo->dtStart().date(), m_dateFormat);
            }

        } else {
            beginName = getDataByFormat(m_scheduleInfo->dtStart().date(), m_dateFormat) + " " + m_scheduleInfo->dtStart().time().toString(m_timeFormat);
            endName = getDataByFormat(m_scheduleInfo->dtEnd().date(), m_dateFormat) + " " + m_scheduleInfo->dtEnd().time().toString(m_timeFormat);
            showTime = beginName + " ~ " + endName;
        }
        m_timeLabel->setText(showTime);
    }
    slotAutoFeed();
}

QString CMyScheduleView::getDataByFormat(const QDate &date, QString format)
{
    QString name = date.toString(format);
    if (m_scheduleInfo->lunnar()) {
        //接入农历时间
        name += gLunarManager->getHuangLiShortName(date);
    }
    return name;
}

/**
 * @brief CMyScheduleView::slotBtClick      按钮点击事件
 * @param buttonIndex
 * @param buttonName
 */
void CMyScheduleView::slotBtClick(int buttonIndex, const QString &buttonName)
{
    Q_UNUSED(buttonName);
    if (buttonIndex == 0) {
        //删除日程
        if (CScheduleOperation(m_scheduleInfo->scheduleTypeID(), this).deleteSchedule(m_scheduleInfo)) {
            accept();
        }
        return;
    }
    if (buttonIndex == 1) {
        //编辑日程
        CScheduleDlg dlg(0, this);
        dlg.setData(m_scheduleInfo);
        if (dlg.exec() == DDialog::Accepted) {
            accept();
        }
        return;
    }
}

/**
 * @brief CMyScheduleView::initUI       界面初始化
 */
void CMyScheduleView::initUI()
{
    //在点击任何对话框上的按钮后不关闭对话框，保证关闭子窗口时不被一起关掉
    setOnButtonClickedClose(false);

    m_Title = new QLabel(this);
    m_Title->setFixedSize(108, 51);
    m_Title->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_Title, DFontSizeManager::T5, QFont::DemiBold);
    //设置日期图标
    QIcon t_icon(CDynamicIcon::getInstance()->getPixmap());
    setIcon(t_icon);
    m_Title->setText(tr("My Event"));
    m_Title->move(137, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    area = new QScrollArea(this);
    //设置日程显示区域不能选中
    area->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    area->setFrameShape(QFrame::NoFrame);
    area->setFixedWidth(363);
    area->setBackgroundRole(QPalette::Background);
    area->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    area->setWidgetResizable(true);
    area->setAlignment(Qt::AlignCenter);

    m_scheduleLabel = new QLabel(this);
    m_scheduleLabel->setTextFormat(Qt::PlainText); //纯文本格式
    m_scheduleLabel->installEventFilter(this);
    m_scheduleLabel->setFixedWidth(330);
    m_scheduleLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_scheduleLabel, DFontSizeManager::T6);
    labelF.setWeight(QFont::Medium);
    m_scheduleLabel->setFont(labelF);

    area->setWidget(m_scheduleLabel);
    mainLayout->addWidget(area);

    m_timeLabel = new DLabel(this);
    m_timeLabel->setFixedHeight(26);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    QFont timeFont;
    timeFont.setWeight(QFont::Normal);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setFixedWidth(363);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_timeLabel);

    //如果为节假日日程
    if (CScheduleOperation::isFestival(m_scheduleInfo)) {
        addButton(tr("OK", "button"), false, DDialog::ButtonNormal);
        QAbstractButton *button_ok = getButton(0);
        button_ok->setFixedSize(360, 36);
    } else {
        addButton(tr("Delete", "button"), false, DDialog::ButtonNormal);
        addButton(tr("Edit", "button"), false, DDialog::ButtonRecommend);
        for (int i = 0; i < buttonCount(); i++) {
            QAbstractButton *button = getButton(i);
            button->setFixedSize(165, 36);
        }
        //TODO:如果为不可修改日程则设置删除按钮无效
    }

    //这种中心铺满的weiget，显示日程标题和时间的控件
    DWidget *centerWidget = new DWidget(this);
    centerWidget->setLayout(mainLayout);
    //获取widget的调色板
    DPalette centerWidgetPalette = centerWidget->palette();
    //设置背景色为透明
    centerWidgetPalette.setColor(DPalette::Background, Qt::transparent);
    centerWidget->setPalette(centerWidgetPalette);
    //添加窗口为剧中对齐
    addContent(centerWidget, Qt::AlignCenter);
}

void CMyScheduleView::initConnection()
{
    //关联主题改变事件
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this,
                     &CMyScheduleView::setLabelTextColor);
    //如果为节假日日程
    if (CScheduleOperation::isFestival(m_scheduleInfo)) {
        connect(this, &DDialog::buttonClicked, this, &CMyScheduleView::close);
    } else {
        connect(this, &DDialog::buttonClicked, this, &CMyScheduleView::slotBtClick);
    }
    QObject::connect(qGuiApp, &DApplication::fontChanged, this, &CMyScheduleView::slotAutoFeed);

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QLatin1String("ESC")));
    connect(shortcut, SIGNAL(activated()), this, SLOT(close()));
    connect(gAccountManager, &AccountManager::signalAccountStateChange, this, &CMyScheduleView::slotAccountStateChange);
}
