// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "popupdialog.h"
#include "uitools.h"
#include "queries.h"

#include <DWidget>
#include <DFontSizeManager>
#include <DCheckBox>
#include <DRadioButton>
#include <QEvent>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QDebug>
#include <DPasswordEdit>
#include <QFileInfo>
const int g_nTextLength = 255;
const int g_nDlgWidth = 380;
const int g_nDlgHeight = 178;
const int g_nLableHeight = 20;
const int g_nLineEditHeight = 36;

TipDialog::TipDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);
    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

TipDialog::~TipDialog()
{

}

int TipDialog::showDialog(const QString &strDesText, const QString btnMsg, ButtonType btnType, const QString &strToolTip)
{
    m_strDesText = strDesText;
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setObjectName("ContentLabel");
    pDesLbl->setFixedWidth(340);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setAlignment(Qt::AlignCenter);
    // 设置提示信息
    if (!strToolTip.isEmpty())
        pDesLbl->setToolTip(strToolTip);

    // 确定按钮
    addButton(btnMsg, true, btnType);
    addContent(pDesLbl, Qt::AlignHCenter); // 使用Qt::AlignHCenter效果最好

    autoFeed(pDesLbl);

    return exec();
}

void TipDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_strDesText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight + height_lable); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iDialogOldHeight = height();
}

void TipDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            autoFeed(p);
        }
    }
    DDialog::changeEvent(event);
}

SimpleQueryDialog::SimpleQueryDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);

    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

SimpleQueryDialog::~SimpleQueryDialog()
{

}

int SimpleQueryDialog::showDialog(const QString &strDesText, const QString btnMsg1, DDialog::ButtonType btnType1, const QString btnMsg2, DDialog::ButtonType btnType2, const QString btnMsg3, DDialog::ButtonType btnType3)
{
    m_strDesText = strDesText;
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setObjectName("ContentLabel");
    pDesLbl->setFixedWidth(340);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setAlignment(Qt::AlignCenter);

    // 确定按钮
    addButton(btnMsg1, true, btnType1);
    addButton(btnMsg2, true, btnType2);
    if (btnMsg3.size() > 0) {
        addButton(btnMsg3, true, btnType3);
    }

    addContent(pDesLbl, Qt::AlignHCenter);

    autoFeed(pDesLbl);

    return exec();
}

void SimpleQueryDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_strDesText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight + height_lable); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iDialogOldHeight = height();
}

void SimpleQueryDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            autoFeed(p);
        }
    }

    DDialog::changeEvent(event);
}

OverwriteQueryDialog::OverwriteQueryDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);
    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

OverwriteQueryDialog::~OverwriteQueryDialog()
{

}

void OverwriteQueryDialog::showDialog(QString file, bool bDir)
{
    m_strFilesname = file;
    DLabel *strlabel = new DLabel;
    strlabel->setObjectName("NameLabel");
    strlabel->setFixedWidth(340);
    strlabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Normal);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setObjectName("ContentLabel");
    strlabel2->setFixedWidth(340);
    strlabel2->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);
    strlabel2->setForegroundRole(DPalette::ToolTipText);

    if (bDir) {
        // 文件夹提示
        strlabel2->setText(QObject::tr("Another folder with the same name already exists, replace it?"));
        m_strDesText = strlabel2->text();
        addButton(QObject::tr("Skip", "button"));
        addButton(QObject::tr("Merge", "button"), true, DDialog::ButtonWarning);
    } else {
        // 文件提示
        strlabel2->setText(QObject::tr("Another file with the same name already exists, replace it?"));
        m_strDesText = strlabel2->text();
        addButton(QObject::tr("Skip", "button"));
        addButton(QObject::tr("Replace", "button"), true, DDialog::ButtonWarning);
    }

    DCheckBox *checkbox = new DCheckBox(QObject::tr("Apply to all"));
    DFontSizeManager::instance()->bind(checkbox, DFontSizeManager::T6, QFont::Medium);
    checkbox->setAccessibleName("Applyall_btn");

    addContent(strlabel2, Qt::AlignHCenter);
    addContent(strlabel, Qt::AlignHCenter);
    addContent(checkbox, Qt::AlignHCenter);

    //setTabOrder需放在布局最后，否则不生效
    this->setTabOrder(checkbox, this->getButton(0));
    this->setTabOrder(this->getButton(0), this->getButton(1));

    autoFeed(strlabel, strlabel2);

    const int mode = exec();
    m_applyAll = checkbox->isChecked();  // 是否应用到全部文件

    if (0 == mode) {
        m_retType = OR_Skip;  // 跳过
    } else if (1 == mode) {
        m_retType = OR_Overwrite;  // 替换
    } else {
        m_retType = OR_Cancel;   // 取消
    }
}

Overwrite_Result OverwriteQueryDialog::getDialogResult()
{
    return m_retType;
}

bool OverwriteQueryDialog::getApplyAll()
{
    return m_applyAll;
}

void OverwriteQueryDialog::autoFeed(DLabel *label1, DLabel *label2)
{
    NewStr newstr = autoCutText(m_strDesText, label2);
    label2->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label2->setFixedHeight(height_lable);

    // 字符串太长的情况下用中间使用...
    QFont font;
    QFontMetrics elideFont(font);
    label1->setText(elideFont.elidedText(m_strFilesname, Qt::ElideMiddle, 340));

    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iLabelOld1Height - m_iCheckboxOld1Height + height_lable + 2 * newstr.fontHeifht); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iLabelOld1Height = newstr.fontHeifht;
    m_iCheckboxOld1Height = newstr.fontHeifht;
    m_iDialogOldHeight = height();
}

void OverwriteQueryDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            Dtk::Widget::DLabel *pNameLabel = findChild<Dtk::Widget::DLabel *>("NameLabel");
            if (nullptr != pNameLabel) {
                autoFeed(pNameLabel, p);
            }
        }
    }

    DDialog::changeEvent(event);
}


ConvertDialog::ConvertDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380); // 提示框宽度固定

    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

ConvertDialog::~ConvertDialog()
{

}

QStringList ConvertDialog::showDialog()
{
    DLabel *strlabel = new DLabel;
    strlabel->setObjectName("ContentLabel");
    strlabel->setFixedWidth(340); // 宽度固定
    strlabel->setText(tr("Changes to archives in this file type are not supported. Please convert the archive format to save the changes."));
    m_strDesText = strlabel->text();
    strlabel->setAlignment(Qt::AlignCenter);
    strlabel->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);

    DLabel *strlabel2 = new DLabel;
    strlabel2->setMinimumSize(QSize(112, 20));
    strlabel2->setText(tr("Convert the format to:"));
    strlabel2->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);

    DRadioButton *zipBtn = new DRadioButton("ZIP");
    zipBtn->setChecked(true);
    DRadioButton *_7zBtn = new DRadioButton("7Z");
    _7zBtn->setChecked(false);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addStretch();
    labelLayout->addWidget(strlabel2);
    labelLayout->setSpacing(20);
    labelLayout->addWidget(zipBtn);
    labelLayout->setSpacing(20);
    labelLayout->addWidget(_7zBtn);
    labelLayout->addStretch();
    labelLayout->setContentsMargins(0, 0, 0, 0);

    DWidget *widget = new DWidget();
    widget->setLayout(labelLayout);
    addContent(strlabel, Qt::AlignHCenter); // 使用Qt::AlignHCenter效果最好
    addContent(widget, Qt::AlignHCenter);
    // 添加取消和转换按钮
    addButton(tr("Cancel", "button"));
    addButton(tr("Convert", "button"), true, DDialog::ButtonRecommend);

    // 设置焦点顺序
    setTabOrder(zipBtn, _7zBtn);
    setTabOrder(_7zBtn, getButton(0));
    setTabOrder(getButton(0), getButton(1));

    bool isZipConvert = true;
    bool is7zConvert = false;
    QString convertType;

    // 转换zip格式
    connect(zipBtn, &DRadioButton::toggled, this, [ =, &isZipConvert]() {
        isZipConvert = zipBtn->isChecked();
        qInfo() << "zip" << isZipConvert;
    });

    // 转换为7z格式
    connect(_7zBtn, &DRadioButton::toggled, this, [ =, &is7zConvert]() {
        is7zConvert = _7zBtn->isChecked();
        qInfo() << "7z" << is7zConvert;
    });

    autoFeed(strlabel);

    const int mode = exec();

    QStringList typeList;
    if (QDialog::Accepted == mode) {
        if (isZipConvert) {
            typeList << "true" << "zip";
        } else if (is7zConvert) {
            typeList << "true" << "7z";
        }
    } else {
        typeList << "false" << "none";
    }

    return typeList;
}

void ConvertDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_strDesText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iLabelOld1Height + height_lable + newstr.fontHeifht); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iLabelOld1Height = newstr.fontHeifht;
    m_iDialogOldHeight = height();
}

void ConvertDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            autoFeed(p);
        }
    }

    DDialog::changeEvent(event);
}


AppendDialog::AppendDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380); // 提示框宽度固定

    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

AppendDialog::~AppendDialog()
{

}

int AppendDialog::showDialog(bool bMultiplePassword)
{
    // 标题
    DLabel *pTitleLbl = new DLabel(this);
    pTitleLbl->setObjectName("ContentLabel");
    pTitleLbl->setFixedWidth(340);
    pTitleLbl->setForegroundRole(DPalette::ToolTipText);
//    pTitleLbl->setWordWrap(true);
    DFontSizeManager::instance()->bind(pTitleLbl, DFontSizeManager::T6, QFont::Medium);
    pTitleLbl->setText(tr("Add files to the current archive"));
    pTitleLbl->setAlignment(Qt::AlignCenter);
    m_strDesText = pTitleLbl->text();

    // 勾选密码
    DCheckBox *pSelCkb = new DCheckBox(tr("Use password"), this);
    DFontSizeManager::instance()->bind(pSelCkb, DFontSizeManager::T6, QFont::Medium);

    // 密码输入框
    DPasswordEdit *pPasswordEdit = new DPasswordEdit(this);
    pPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false); //隐藏密码时不能输入中文
    pPasswordEdit->setFocusPolicy(Qt::StrongFocus);
    pPasswordEdit->setMinimumHeight(36);
    pPasswordEdit->setFixedWidth(280);
    pPasswordEdit->setVisible(false);

    // 不支持多密码的格式不可以勾选密码框
    if (!bMultiplePassword) {
        pSelCkb->setEnabled(false);
    }

    // 密码选择框勾选之后显示密码输入框
    connect(pSelCkb, &DCheckBox::clicked, this, [ & ]() {
        if (pSelCkb->checkState() == Qt::Checked) {
            pPasswordEdit->setVisible(true);
            m_bPasswordVisible = true;
            setFixedHeight(m_iDialogOldHeight + 42);
        } else {
            pPasswordEdit->clear();     // 不勾选加密时，清空密码
            pPasswordEdit->setVisible(false);
            m_bPasswordVisible = false;
            setFixedHeight(m_iDialogOldHeight - 42);
        }
        m_iDialogOldHeight = height();
//        adjustSize();
    });

    // 布局
    addContent(pTitleLbl, Qt::AlignHCenter);
//    addSpacing(5);
    addContent(pSelCkb, Qt::AlignHCenter);
    addSpacing(5);
    addContent(pPasswordEdit, Qt::AlignHCenter);
//    addSpacing(10);

    addButton(QObject::tr("Cancel", "button"), true, DDialog::ButtonNormal);
    addButton(QObject::tr("OK", "button"), true, DDialog::ButtonRecommend);

    autoFeed(pTitleLbl);

    int iMode = exec();

    // 接收加密
    m_strPassword.clear();
    if (DDialog::Accepted == iMode) {
        m_strPassword = pPasswordEdit->text();
    }

    return iMode;
}

QString AppendDialog::password()
{
    return m_strPassword;
}

void AppendDialog::autoFeed(DLabel *pLabel)
{
    NewStr newstr = autoCutText(m_strDesText, pLabel);
    pLabel->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    pLabel->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iCheckboxOld1Height + height_lable + newstr.fontHeifht); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iCheckboxOld1Height = newstr.fontHeifht;
    m_iDialogOldHeight = height();
}

void AppendDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("ContentLabel");
        if (nullptr != p) {
            autoFeed(p);
        }
    }
    DDialog::changeEvent(event);
}

RenameDialog::RenameDialog(QWidget *parent)
    : DDialog(parent)
{
    m_nOkBtnIndex = -1;
    m_lineEdit = NULL;
    setFixedSize(g_nDlgWidth, g_nDlgHeight); // 提示框宽度固定
    m_isDirectory = false;
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    setIcon(pixmap);
}

RenameDialog::~RenameDialog()
{
    if(m_lineEdit)  {
        delete m_lineEdit;
        m_lineEdit = NULL;
    }
}

int RenameDialog::showDialog(const QString &strReName, const QString &strAlias, bool isDirectory, bool isRepeat)
{
    if(m_lineEdit == NULL) {//创建DLineEdit,设定显示规则。
        m_lineEdit = new DLineEdit(this);
        //输入字符格式与文管保持一致
        connect(m_lineEdit, &DLineEdit::textChanged,[=](){
            QString name = "(^\\s+|[/\\\\:*\"'?<>|\r\n\t])";
            QString text = m_lineEdit->text();
            m_lineEdit->setText(text.remove(QRegularExpression(name)));
            if(m_nOkBtnIndex >= 0) { //重命名输入文字为空时确认按钮不可点击，输入文字后恢复
                if(text.isNull()||text.isEmpty()){
                    getButton(m_nOkBtnIndex)->setEnabled(false);
                } else {
                    getButton(m_nOkBtnIndex)->setEnabled(true);
                }
            }
        });
    }
    DLabel *strlabel = new DLabel;
    strlabel->setObjectName("ContentLabel");
    strlabel->setFixedSize(g_nDlgWidth, g_nLableHeight); // 宽度固定
    strlabel->setText(tr("Rename"));
    strlabel->setAlignment(Qt::AlignCenter);
    strlabel->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);

    m_lineEdit->setMaximumHeight(g_nLineEditHeight);
    if(strAlias.isEmpty() || strAlias.isNull()) {
        if(isDirectory) {
            m_lineEdit->setText(QFileInfo(strReName).fileName());
        } else {
            m_lineEdit->setText(QFileInfo(strReName).completeBaseName());
        }
    } else {
        if(isDirectory) {
            m_lineEdit->setText(QFileInfo(strAlias).fileName());
        } else {
            m_lineEdit->setText(QFileInfo(strAlias).completeBaseName());
        }
    }
    m_isDirectory = isDirectory;
    m_strName = strReName;

//    m_lineEdit->setClearButtonEnabled(true);
    //限制输入最大字符长度255
    if(m_isDirectory) {
        m_lineEdit->lineEdit()->setMaxLength(g_nTextLength);
    } else {
        QString strSuffix = QFileInfo(m_strName).suffix();
        if(strSuffix.isNull() || strSuffix.isEmpty()) {
            m_lineEdit->lineEdit()->setMaxLength(g_nTextLength);
        } else {
            int nLength = g_nTextLength-QFileInfo(m_strName).suffix().length()-1;
            if(nLength < 0) nLength = 0;
            m_lineEdit->lineEdit()->setMaxLength(nLength);
        }
    }
    DFontSizeManager::instance()->bind(m_lineEdit, DFontSizeManager::T6, QFont::Medium);


    QVBoxLayout *labelLayout = new QVBoxLayout;
    labelLayout->addStretch();
    labelLayout->addWidget(m_lineEdit);
    labelLayout->addStretch();
    labelLayout->setContentsMargins(0, 0, 0, 0);

    DWidget *widget = new DWidget();
    widget->setFixedWidth(360);
    widget->setLayout(labelLayout);
    addContent(strlabel, Qt::AlignHCenter); // 使用Qt::AlignHCenter效果最好
    addContent(widget, Qt::AlignHCenter);
    // 添加取消和转换按钮
    addButton(tr("Cancel", "button"));
    m_nOkBtnIndex = addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);

    // 设置焦点顺序
    setTabOrder(m_lineEdit, getButton(0));
    setTabOrder(getButton(0), getButton(1));
    if(!isRepeat) {
        m_lineEdit->setFocus();
    } else {
        strlabel->setFocus();
        m_lineEdit->setAlert(true);
        m_lineEdit->showAlertMessage(tr("The name already exists"));
    }
    return  exec();

}

QString RenameDialog::getNewNameText() const
{
    if(!m_lineEdit) return "";
    if(m_lineEdit->text().isNull() || m_lineEdit->text().isEmpty()) {
        return QFileInfo(m_strName).absoluteFilePath();
    }
    if(m_isDirectory) {
        return QFileInfo(m_strName).path() + QDir::separator() + m_lineEdit->text();
    } else {
        QString strSuffix = QFileInfo(m_strName).suffix();
        if(strSuffix.isNull() || strSuffix.isEmpty()) {
            return QFileInfo(m_strName).path() + QDir::separator() + m_lineEdit->text();
        } else {
            return QFileInfo(m_strName).path() + QDir::separator() + m_lineEdit->text() +"."+ QFileInfo(m_strName).suffix();
        }
    }
}

