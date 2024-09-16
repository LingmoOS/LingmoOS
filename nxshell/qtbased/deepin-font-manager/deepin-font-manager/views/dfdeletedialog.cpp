// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfdeletedialog.h"
#include "utils.h"
#include "globaldef.h"
#include "dfontmgrmainwindow.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DCheckBox>

#include <QHBoxLayout>
#include <QKeyEvent>

struct NewStr {
    QStringList strList; //每行的字符串
    QString resultStr; //添加'\n'后的字符串
    int fontHeifht = 0; //当前字体的高度
};

/**
 * @brief autoCutText 根据字符串长度和pDesLbl宽度适当添加'\n'
 * @param text
 * @param pDesLbl
 * @return
 */
NewStr autoCutText(const QString &text, DLabel *pDesLbl)
{
    if (text.isNull() || nullptr == pDesLbl) {
        return NewStr();
    }

    QFont font; // 应用使用字体对象
    QFontMetrics font_label(font);
    QString strText = text;
    int titlewidth = font_label.width(strText);
    QString str;
    NewStr newstr;
    int width = pDesLbl->width();
    if (titlewidth < width) {
        newstr.strList.append(strText);
        newstr.resultStr += strText;
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);

            if (font_label.width(str) > width) { //根据label宽度调整每行字符数
                str.remove(str.count() - 1, 1);
                newstr.strList.append(str);
                newstr.resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        newstr.strList.append(str);
        newstr.resultStr += str;
    }
    newstr.fontHeifht = font_label.height();
    return newstr;
}

/*************************************************************************
 <Function>      DFDeleteDialog
 <Description>   构造函数：此类为字体删除确认页面，主要功能为确认删除字体和取消删除字体
 <Author>
 <Input>
    <param1>     win               Description:主页面实例对象
    <param2>     deleteCnt         Description:待删除字体数量
    <param3>     systemCnt         Description:选中系统字体的数量
    <param3>     hasCurrent        Description:是否包含当前系统在用字体
    <param3>     parent            Description:父对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFDeleteDialog::DFDeleteDialog(DFontMgrMainWindow *win, int deleteCnt, int systemCnt, bool hasCurrent, QWidget *parent)
    : DDialog(parent)
    , m_mainWindow(win)
    , m_deleteCnt(deleteCnt)
    , m_systemCnt(systemCnt)
    , m_deleting(false)
    , m_hasCurFont(hasCurrent)
{
    initUI();
    initConnections();
    setTheme();
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化字体删除待确认页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initUI()
{
    setFixedWidth(DEFAULT_WINDOW_W);
    setIcon(QIcon::fromTheme("deepin-font-manager"));

    initMessageTitle();
    initMessageDetail();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->addWidget(messageTitle, 0, Qt::AlignCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(messageDetail, 0, Qt::AlignCenter);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    messageDetail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainFrame->setLayout(mainLayout);

    //#000794 解决显示不全的问题
    messageTitle->setMinimumWidth(DEFAULT_WINDOW_W - 20);
    messageDetail->setMinimumWidth(DEFAULT_WINDOW_W - 20);

    addContent(mainFrame);

    insertButton(0, DApplication::translate("DFDeleteDialog", "Cancel", "button"), false, ButtonNormal);
    insertButton(1, DApplication::translate("DeleteConfirmDailog", "Delete", "button"), true, ButtonWarning);
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化用于判断删除或取消操作的信号和槽的链接函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initConnections()
{
    //关闭删除确认对话框并且没有点击"确认"按钮时,取消删除操作
    connect(this, &DFDeleteDialog::closed, this, [ = ]() {
        if (m_mainWindow != nullptr && !m_deleting) {
            qDebug() << "cancel delte";
            m_mainWindow->cancelDelete();
            emit m_signalManager->clearRecoverList();
        }
    });

    connect(this, &DFDeleteDialog::buttonClicked, this, [=](int index, const QString &text){
        switch (index) {
        case 0:
            reject();
            close();
            emit m_signalManager->cancelDel();
            break;

        case 1:
            if (m_deleting)
                return;
            m_deleting = true;
            accept();
            close();
            break;

        default:
            break;
        }
    });

    connect(qApp, &DApplication::fontChanged, this, &DFDeleteDialog::onFontChanged);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &DFDeleteDialog::setTheme);
}

/*************************************************************************
 <Function>      initMessageTitle
 <Description>   初始化页面提示信息标题
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initMessageTitle()
{
    messageTitle = new DLabel(this);
    messageTitle->setText(DApplication::translate("DeleteConfirmDailog", "Are you sure you want to delete %1 font(s)?").arg(m_deleteCnt));

    /* Bug#21515 UT000591*/
    messageTitle->setFixedWidth(DEFAULT_WINDOW_W - 22);
    messageTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    messageTitle->setWordWrap(true);
    messageTitle->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageTitle, DFontSizeManager::T6, QFont::Medium);
}

/*************************************************************************
 <Function>      initMessageDetail
 <Description>   初始化页面提示信息内容
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initMessageDetail()
{
    messageDetail = new DLabel(this);

    if (m_systemCnt == 0 && !m_hasCurFont) {
        if (m_deleteCnt == 1) {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "This font will not be available to applications"));
        } else {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "These fonts will not be available to applications"));
        }
    } else if (m_systemCnt > 0 && !m_hasCurFont) {
        messageDetail->setText(DApplication::translate("DeleteConfirmDailog",
                                                       "The other %1 system fonts cannot be deleted").arg(m_systemCnt));
    } else if (m_systemCnt == 0 && m_hasCurFont) {
        messageDetail->setText(DApplication::translate("DeleteConfirmDailog",
                                                       "The font \"%1\" in use cannot be deleted").arg(m_mainWindow->getPreviewListView()->getCurFontStrName()));
    } else {
        messageDetail->setText(DApplication::translate("DeleteConfirmDailog",
                                                       "The other %1 system fonts and the font \"%2\" in use cannot be deleted").arg(m_systemCnt)
                               .arg(m_mainWindow->getPreviewListView()->getCurFontStrName()));
    }

    /* Bug#21515 UT000591*/
    messageDetail->setFixedWidth(DEFAULT_WINDOW_W - 22);
    messageDetail->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    messageDetail->setWordWrap(true);
    messageDetail->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageDetail, DFontSizeManager::T6, QFont::Medium);
}

/*************************************************************************
 <Function>      onFontChanged
 <Description>   适应系统字体变化
 <Author>
 <Input>
    <param1>     font            Description:当前系统在用字体
    <param2>     null            Description:
    <param3>     null            Description:
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::onFontChanged(const QFont &font)
{
    Q_UNUSED(font);
    /* Bug#20953 #21069  UT000591 */
    messageDetail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(sizeHint());
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   响应键盘press事件中的esc按键
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::keyPressEvent(QKeyEvent *event)
{
    bool received = false;
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
        received = true;
    }
    if (event->key() == Qt::Key_Return) {
        if (!getButton(0)->hasFocus() && !getButton(1)->hasFocus()) {
            getButton(1)->click();
            received = true;
        }
    }
    if (!received)
        QDialog::keyPressEvent(event);
}

/*************************************************************************
 <Function>      setTheme
 <Description>   根据主题设置页面颜色
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::setTheme()
{
    // 根据主题设置文字颜色
    DPalette pamessageTitle = DApplicationHelper::instance()->palette(messageTitle);
    DPalette pamessageDetail = DApplicationHelper::instance()->palette(messageDetail);
    QColor pamessageTitleColor = pamessageTitle.color(DPalette::Active, DPalette::BrightText);
    QColor pamessageDetailColor = pamessageDetail.color(DPalette::Active, DPalette::BrightText);
    if (DApplicationHelper::DarkType == DApplicationHelper::instance()->themeType()) {
        pamessageTitleColor.setAlphaF(1.0);
        pamessageDetailColor.setAlphaF(0.7);
    } else {
        pamessageTitleColor.setAlphaF(0.9);
        pamessageDetailColor.setAlphaF(0.7);
    }
    pamessageTitle.setColor(DPalette::Active, DPalette::WindowText, pamessageTitleColor);
    DApplicationHelper::instance()->setPalette(messageTitle, pamessageTitle);
    pamessageDetail.setColor(DPalette::Active, DPalette::WindowText, pamessageDetailColor);
    DApplicationHelper::instance()->setPalette(messageDetail, pamessageDetail);
}

DFHandleTTCDialog::DFHandleTTCDialog(DFontMgrMainWindow *win, QString &file, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_mainWindow(win)
    , fontset(file)
{

}

bool DFHandleTTCDialog::getDeleting()
{
    return m_confirm;
}

bool DFHandleTTCDialog::getAapplyToAll()
{
    return m_bAapplyToAll;
}

bool DFHandleTTCDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == applyAllCkb) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                applyAllCkb->setChecked(!applyAllCkb->isChecked()); // 应用全部
                m_bAapplyToAll = applyAllCkb->isChecked();
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return DFontBaseDialog::eventFilter(watched, event);
    }
}

int DFHandleTTCDialog::execDialog()
{
    // 因为使用了adjustSize()，在exec之前dialog的大小未知，
    // 所以需要延时5ms(经验值)后moveToCenter才准确
    QTimer::singleShot(5, this, [&]() {
        moveToCenter();
    });

    return exec();
}

void DFHandleTTCDialog::keyPressEvent(QKeyEvent *event)
{
    bool received = false;
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
        received = true;
    }
    if (!received)
        DFontBaseDialog::keyPressEvent(event);
}

void DFHandleTTCDialog::initUI()
{
    setFixedWidth(DEFAULT_WINDOW_W);

    initMessageTitle();
    initMessageDetail();
    setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
    QLayout *buttonsLayout = initBottomButtons();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->addWidget(messageTitle, 0, Qt::AlignCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(applyAllCkb, 0, Qt::AlignCenter);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(buttonsLayout);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainFrame->setLayout(mainLayout);

    //#000794 解决显示不全的问题
    messageTitle->setMinimumWidth(DEFAULT_WINDOW_W - 40); //删除/禁用/启用字体集提示窗口中文字边距设为20

    addContent(mainFrame);
    applyAllCkb->installEventFilter(this);
}

void DFHandleTTCDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        reject();
        close();
    });
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        m_confirm = true;
        accept();
        close();
    });

    connect(applyAllCkb, &DCheckBox::clicked, this, [ = ]() {
        m_bAapplyToAll = applyAllCkb->isChecked();
    });
}

void DFHandleTTCDialog::initMessageTitle()
{
    messageTitle = new DLabel(this);
    messageTitle->setObjectName("messageTitle");
    setMessageTitleText();

    /* Bug#21515 UT000591*/
    messageTitle->setFixedWidth(DEFAULT_WINDOW_W - 40); //删除/禁用/启用字体集提示窗口中文字边距设为20
    messageTitle->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageTitle, DFontSizeManager::T6, QFont::Medium);
    messageTitle->setForegroundRole(DPalette::WindowText);

    autoFeed(messageTitle);
}

void DFHandleTTCDialog::initMessageDetail()
{
    applyAllCkb = new DCheckBox(tr("Apply to all selected font families"), this);
    applyAllCkb->setAccessibleName("Applyall_btn");
    DFontSizeManager::instance()->bind(applyAllCkb, DFontSizeManager::T6, QFont::Medium);
}

QLayout *DFHandleTTCDialog::initBottomButtons()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedSize(170, 36);
    m_cancelBtn->setText(tr("Cancel"));

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedSize(170, 36);
    setConfirmBtnText();

    DVerticalLine *verticalSplite = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplite);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalSplite->setPalette(pa);
    verticalSplite->setBackgroundRole(QPalette::Background);
    verticalSplite->setAutoFillBackground(true);
    verticalSplite->setFixedSize(3, 28);

    layout->addWidget(m_cancelBtn);
    layout->addSpacing(9);
    layout->addWidget(verticalSplite);
    layout->addSpacing(9);
    layout->addWidget(m_confirmBtn);

    return layout;
}

void DFHandleTTCDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_messageTitleText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        m_iDialogOldHeight = height();
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight + height_lable); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
}

void DFHandleTTCDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("messageTitle");
        if (nullptr != p) {
            autoFeed(p);
        }
    }
    DFontBaseDialog::changeEvent(event);
}

DFDeleteTTCDialog::DFDeleteTTCDialog(DFontMgrMainWindow *win, QString &file, QWidget *parent)
    : DFHandleTTCDialog(win, file, parent)
{
    initUI();
    initConnections();
}

void DFDeleteTTCDialog::setConfirmBtnText()
{
    if (m_confirmBtn) {
        m_confirmBtn->setText(tr("Delete", "button"));
    }
}

void DFDeleteTTCDialog::setMessageTitleText()
{
    m_messageTitleText = (tr("%1 is a font family, if you proceed, all fonts in it will be deleted").arg(fontset));
}

DFDisableTTCDialog::DFDisableTTCDialog(DFontMgrMainWindow *win, QString &file, bool &isEnable, QWidget *parent)
    : DFHandleTTCDialog(win, file, parent)
    , m_isEnable(isEnable)
{
    initUI();
    initConnections();
}
void DFDisableTTCDialog::setMessageTitleText()
{
    if (m_isEnable) {
        m_messageTitleText = (tr("%1 is a font family, if you proceed, all fonts in it will be enabled").arg(fontset));
    } else {
        m_messageTitleText = (tr("%1 is a font family, if you proceed, all fonts in it will be disabled").arg(fontset));
    }
}

void DFDisableTTCDialog::setConfirmBtnText()
{
    if (!m_confirmBtn) {
        return;
    }

    if (m_isEnable) {
        m_confirmBtn->setText(tr("Enable", "button"));
    } else {
        m_confirmBtn->setText(tr("Disable", "button"));
    }
}
