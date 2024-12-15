// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduletypeeditdlg.h"
#include "scheduledatamanage.h"
#include "cscheduleoperation.h"
#include "configsettings.h"
#include "units.h"

#include <DFrame>
#include <DTitlebar>
#include <DIconButton>

#include <QLabel>
#include <QRadioButton>
#include <QFormLayout>

ScheduleTypeEditDlg::ScheduleTypeEditDlg(QWidget *parent)
    : DDialog(parent)
    , m_title(tr("New event type"))
    , m_dialogType(DialogNewType)
{
    init();
}

ScheduleTypeEditDlg::ScheduleTypeEditDlg(const DScheduleType &jobTypeOld, QWidget *parent)
    : DDialog(parent)
    , m_jobTypeOld(jobTypeOld)
    , m_jobTypeNew(jobTypeOld)
    , m_title(tr("Edit event type"))
    , m_dialogType(DialogEditType)

{
    init();
}

ScheduleTypeEditDlg::ScheduleTypeEditDlg(const DialogType &type, QWidget *parent)
    : DDialog(parent)
{
    switch (type) {
    case DialogImportType:
        m_title = tr("Import ICS file");
        m_dialogType = DialogImportType;
        break;
    default:
        m_title = tr("New event type");
        m_dialogType = DialogNewType;
        break;
    }
    init();
}

DScheduleType ScheduleTypeEditDlg::newJsonType()
{
    m_jobTypeNew.setTypeColor(*m_colorSeletor->getSelectedColorInfo());
    return m_jobTypeNew;
}

void ScheduleTypeEditDlg::setAccount(AccountItem::Ptr account)
{
    m_colorSeletor->resetColorButton(account);

    //将用户上一次选择的自定义颜色添加进去
    QString colorName = CConfigSettings::getInstance()->value("LastUserColor", "").toString();
    if (!colorName.isEmpty()) {
        //设置颜色
        DTypeColor::Ptr typeColor;
        typeColor.reset(new DTypeColor);
        typeColor->setColorID(0);
        typeColor->setColorCode(colorName);
        typeColor->setPrivilege(DTypeColor::PriUser);
        m_colorSeletor->setUserColor(typeColor);
    }
    switch (m_dialogType) {
    case DialogEditType: {
        //编辑日程类型
        //设置颜色
        m_colorSeletor->setSelectedColor(m_jobTypeOld.typeColor());
    } break;
    default: {
        //默认新建日程，选中上一次选中的颜色
        //选中上一次选中的颜色
        QVariant colorId = CConfigSettings::getInstance()->value("LastSysColorTypeNo", -1);
        int colorNum = 0;
        if (colorId.type() == QVariant::Int) {
            //如果是int型表示为旧颜色编号
            colorNum = colorId.toInt();
        } else {
            QString &&colorIdStr = colorId.toString();
            if(colorName.isEmpty() && colorIdStr.isEmpty()){
                colorNum = -1;
            } else if (!colorIdStr.isEmpty()) {
                colorNum = GTypeColor.keys().indexOf(colorIdStr);
            } else {
                colorNum = 9;
            }
        }
        m_colorSeletor->setSelectedColorById(colorNum);
    } break;
    }
}

void ScheduleTypeEditDlg::init()
{
    initView();
    initData();
    //默认焦点在日程名称输入框中
    m_lineEdit->setFocus();
    //先初始化数据再关联信号，初始状态下不提示
    connect(m_lineEdit, &DLineEdit::textChanged, this, &ScheduleTypeEditDlg::slotEditTextChanged);
    connect(m_lineEdit, &DLineEdit::focusChanged, this, &ScheduleTypeEditDlg::slotFocusChanged);
    connect(m_lineEdit, &DLineEdit::editingFinished, this, &ScheduleTypeEditDlg::slotEditingFinished);
    // 在编辑框变动时，设置确认按钮状态
    connect(m_lineEdit, &DLineEdit::textChanged, this, &ScheduleTypeEditDlg::slotCheckConfirmBtn);
    connect(m_fileEdit, &DLineEdit::textChanged, this, &ScheduleTypeEditDlg::slotCheckConfirmBtn);
}

void ScheduleTypeEditDlg::initView()
{
    setFixedSize(QSize(400, 220));

    m_titleLabel = new QLabel(this);
    QFont titlelabelF;
    titlelabelF.setWeight(QFont::DemiBold);
    titlelabelF.setPixelSize(17);
    m_titleLabel->setFixedSize(400, 25);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->move(0, 19);
    m_titleLabel->setFont(titlelabelF);

    m_lineEdit = new DLineEdit();
    m_lineEdit->setClearButtonEnabled(false); //不显示按钮
    m_colorSeletor = new ColorSeletorWidget();
    m_fileEdit = new DFileChooserEdit;

    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setHorizontalSpacing(10);
    formLayout->setVerticalSpacing(20);
    formLayout->addRow(tr("Name:"), m_lineEdit);
    formLayout->addRow(tr("Color:"), m_colorSeletor);

    if (m_dialogType == DialogImportType){
        setFixedSize(QSize(400, 300));
        auto icsLabel = new QLabel(tr("<a href='https://wikipedia.org/wiki/ICalendar'>ICS</a> File:"), this);
        icsLabel->setOpenExternalLinks(true);
        formLayout->addRow(icsLabel, m_fileEdit);
    }

    DFrame *gwi = new DFrame(this);
    gwi->setFrameShape(QFrame::NoFrame);
    gwi->setLayout(formLayout);
    gwi->setMinimumWidth(360);
    addContent(gwi, Qt::AlignCenter);
    //添加按钮
    addButton(tr("Cancel", "button"));
    addButton(tr("Save", "button"), false, DDialog::ButtonRecommend);
    for (int i = 0; i < buttonCount(); i++) {
        QAbstractButton *button = getButton(i);
        button->setFixedSize(170, 36);
    }
    connect(this->getButton(0), &QPushButton::clicked, this, &ScheduleTypeEditDlg::slotBtnCancel);
    connect(this->getButton(1), &QPushButton::clicked, this, &ScheduleTypeEditDlg::slotBtnNext);
}

void ScheduleTypeEditDlg::initData()
{
    m_titleLabel->setText(m_title);
    m_lineEdit->setText(m_jobTypeOld.displayName());
    m_typeText = m_jobTypeOld.displayName(); //编辑时要初始化数据
    this->getButton(1)->setEnabled(!m_jobTypeOld.displayName().isEmpty()); //如果是新增，则保存按钮默认不可用
}

void ScheduleTypeEditDlg::slotEditTextChanged(const QString &strName)
{
    QString tStitlename = strName;
    //去除回车字符
    if (tStitlename.contains("\n")) {
        //设置纯文本显示原始内容
        tStitlename.replace("\n", "");
        m_lineEdit->setText(tStitlename);
        return;
    }
    //最大限制20个字符，超出后过滤掉
    if (tStitlename.length() > 20) {
        m_lineEdit->setText(m_typeText);
        return;
    } else {
        m_typeText = tStitlename;
    }

    //文本编辑框中文本改变事件
    //1不能为空，2不能全空格，3不能重名

    if (m_lineEdit->text().isEmpty()) {
        //名称为空，返回
        //内容清空时，消除警告色和提示信息
        m_lineEdit->setAlert(false);
        m_lineEdit->hideAlertMessage();

        return;
    }
    if (tStitlename.trimmed().isEmpty()) {
        //名称为全空格，返回
        m_lineEdit->showAlertMessage(tr("The name can not only contain whitespaces"));
        m_lineEdit->setAlert(true);
        return;
    }
    m_jobTypeNew.setDisplayName(tStitlename);

    m_lineEdit->setAlert(false);
    m_lineEdit->hideAlertMessage();
    return;
}

void ScheduleTypeEditDlg::slotFocusChanged(bool onFocus)
{
    //如果焦点移出,且输入内容为空
    if (!onFocus && m_lineEdit->text().isEmpty()) {
//        emit m_lineEdit->textChanged("");
        emit m_lineEdit->editingFinished();
    }
}

void ScheduleTypeEditDlg::slotBtnCancel()
{
    this->reject();
}

void ScheduleTypeEditDlg::slotBtnNext()
{
    m_jobTypeNew.setTypeColor(*m_colorSeletor->getSelectedColorInfo());
    this->accept();
}

void ScheduleTypeEditDlg::slotEditingFinished()
{
    //如果编辑结束后内容为空则提示
    if (m_lineEdit->text().isEmpty()) {
        //名称为空，返回
        m_lineEdit->showAlertMessage(tr("Enter a name please"));
        m_lineEdit->setAlert(true);
    }
}

// 获取ICS文件路径
QString ScheduleTypeEditDlg::getIcsFile()
{
    return m_fileEdit->text();
}

// 检查并设置‘确认按钮’的状态
void ScheduleTypeEditDlg::slotCheckConfirmBtn()
{
    auto confirmBtn = this->getButton(1);

    if (m_lineEdit->text().isEmpty() || m_lineEdit->isAlert())
    {
        confirmBtn->setEnabled(false);
        return;
    }
    if (m_dialogType == DialogType::DialogImportType)
    {
        if (m_fileEdit->text().isEmpty() || m_fileEdit->isAlert())
        {
            confirmBtn->setEnabled(false);
            return;
        }
    }
    confirmBtn->setEnabled(true);
}
