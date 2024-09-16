// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "renamedialog.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"

#include "accessibility/ac-desktop-define.h"
#include <DGuiApplicationHelper>
#include <DMessageBox>
#include <DLabel>
#include <DFontSizeManager>

#include <QRegExp>
#include <QDebug>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

#define FILENAMEMAXLENGTH 255

RenameDialog::RenameDialog(const QString &filename, QWidget *parent)
    : DDialog(parent)
    , m_filenamepath(filename)
{
    this->setIcon(QIcon::fromTheme("deepin-image-viewer"));
    DWidget *widet = new DWidget(this);
    this->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);  // 标题栏和内容部件间隙设为0，在视觉上缩短标题栏间隙
    addContent(widet);
    m_vlayout = new QVBoxLayout(widet);
    m_labvlayout = new QVBoxLayout();
    m_labvlayout->setContentsMargins(0, 0, 0, 10);
    m_labvlayout->setSpacing(5);
    m_hlayout = new QHBoxLayout();
    m_edtlayout = new QHBoxLayout();
    m_lineedt = new DLineEdit(widet);
    QFrame *line = new QFrame(widet);
    line->setLineWidth(1);
    line->setFixedHeight(25);  // 垂直分割线短于按钮高度
    QLbtoDLabel *labtitle = new QLbtoDLabel();
    okbtn = new DSuggestButton(tr("Confirm"), widet);
    cancelbtn = new DPushButton(tr("Cancel"), widet);
    okbtn->setFixedHeight(35);
    cancelbtn->setFixedHeight(35);
    m_labformat = new DLabel(widet);
    m_vlayout->setContentsMargins(2, 0, 2, 1);

    m_hlayout->addWidget(cancelbtn);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Plain);  // 垂直分割线颜色应为浅灰
    m_hlayout->addWidget(line);
    m_hlayout->addWidget(okbtn);
    m_hlayout->setSpacing(7);  // 保证按钮间距20px
    labtitle->setText(tr("Input a new name"));
    labtitle->setFixedHeight(40);  //增加标签区域显示高度，在视觉上缩短标题栏间隙
    labtitle->setAlignment(Qt::AlignCenter);
    m_labvlayout->addWidget(labtitle);

    m_edtlayout->addWidget(m_lineedt);
    m_lineedt->setFixedHeight(35);

    m_labformat->setEnabled(false);
    m_edtlayout->addWidget(m_labformat);
    m_labformat->setFixedHeight(35);
    m_labvlayout->addLayout(m_edtlayout);
    m_vlayout->addLayout(m_labvlayout);
    m_vlayout->addStretch();

    m_vlayout->addLayout(m_hlayout);
    widet->setLayout(m_vlayout);
    onThemeChanged(DGuiApplicationHelper::instance()->themeType());
    InitDlg();
    m_lineedt->lineEdit()->setFocus();
    int Dirlen = /*m_DirPath.size() +*/ 1 + m_labformat->text().size();
    // 正则表达式排除文管不支持的字符
    // 屏蔽特殊字符 QRegExp rx("[^\\\\//:*?\"<>|]*");
    QRegExp rx("^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*");

    QRegExpValidator *pReg = new QRegExpValidator(rx, this);
    m_lineedt->lineEdit()->setValidator(pReg);
    connect(okbtn, &DSuggestButton::clicked, this, [=] {
        m_filename = m_lineedt->text() + m_labformat->text();
        m_filenamepath = m_DirPath + "/" + m_filename;
        accept();
    });
    connect(cancelbtn, &DPushButton::clicked, this, [=] { reject(); });
    connect(m_lineedt, &DLineEdit::textChanged, this, [=](const QString &arg) {
        setCurrentTip();
        int len = arg.toLocal8Bit().length();
        //修复字符串长度超长会将
        if (len > 256 - Dirlen)
            return;
    });
    connect(m_lineedt, &DLineEdit::textEdited, this, [=](const QString &arg) {
        if (arg.isEmpty()) {
            return;
        }
        if (arg.at(0) == " ") {
            QString str = arg;
            str = str.right(str.size() - 1);

            m_lineedt->lineEdit()->setText(str);
            m_lineedt->lineEdit()->setCursorPosition(0);
        }
        int len = arg.toLocal8Bit().length();
        QString Interceptstr;
        if (len > 256 - Dirlen) {
            int num = 0;
            int i = 0;
            for (; i < arg.size(); i++) {
                if (arg.at(i) >= 0x4e00 && arg.at(i) <= 0x9fa5) {
                    num += 3;
                    if (num >= 256 - Dirlen - 1)
                        break;
                } else if (num < 256 - Dirlen) {
                    num += 1;
                } else {
                    break;
                }
            }
            Interceptstr = arg.left(i);
            m_lineedt->lineEdit()->setText(Interceptstr);
        };
    });
    okbtn->setEnabled(true);
    setObjectName(RENAME_WIDGET);
    setAccessibleName(RENAME_WIDGET);
    m_lineedt->setObjectName(INPUT_EDIT);
    m_lineedt->setAccessibleName(INPUT_EDIT);
    okbtn->setObjectName(OK_BUTTON);
    okbtn->setAccessibleName(OK_BUTTON);
    cancelbtn->setObjectName(CANCEL_BUTTON);
    cancelbtn->setAccessibleName(CANCEL_BUTTON);
    widet->setObjectName(RENAME_CONTENT);
    widet->setObjectName(RENAME_CONTENT);

    setFixedSize(380, 190);
}

QString RenameDialog::GetFilePath()
{
    return m_filenamepath;
}

QString RenameDialog::GetFileName()
{
    return m_filename;
}

void RenameDialog::InitDlg()
{
    QFileInfo fileinfo(m_filenamepath);
    m_DirPath = fileinfo.path();
    m_filename = fileinfo.fileName();
    QString format = fileinfo.suffix();
    QString basename;
    // basename会过滤掉.,那么1.....png就会出现basename为1,completeBaseName不会,修改bug66356
    m_basename = fileinfo.completeBaseName();
    m_lineedt->setText(m_basename);
    m_labformat->setText("." + format);
}

void RenameDialog::setCurrentTip()
{
    QString fileabname = m_DirPath + "/" + m_lineedt->text() + m_labformat->text();
    QFile file(fileabname);
    if (m_filenamepath == fileabname) {
        okbtn->setEnabled(true);
        m_lineedt->hideAlertMessage();
    } else if (file.exists()) {
        okbtn->setEnabled(false);
        m_lineedt->setAlert(true);
        m_lineedt->showAlertMessage(tr("The file already exists, please use another name"), m_lineedt);
    } else if (m_lineedt->text().isEmpty()) {
        okbtn->setEnabled(false);
        m_lineedt->hideAlertMessage();
    } else {
        okbtn->setEnabled(true);
        m_lineedt->hideAlertMessage();
    }
}

void RenameDialog::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

QString RenameDialog::geteElidedText(QFont font, QString str, int MaxWidth)
{
    QFontMetrics fontWidth(font);
    int width = fontWidth.horizontalAdvance(str);
    if (width >= MaxWidth) {
        str = fontWidth.elidedText(str, Qt::ElideMiddle, MaxWidth);
    }
    return str;
}

void RenameDialog::slotsFocusChanged(bool onFocus)
{
    if (!onFocus) {
        m_lineedt->hideAlertMessage();
    }
}

void RenameDialog::onThemeChanged(DGuiApplicationHelper::ColorType theme)
{
    // 文件后缀颜色跟随主题变化
    QPalette palette = m_labformat->palette();
    if (theme == DGuiApplicationHelper::ColorType::DarkType) {
        const QColor prefixColor = QColor(179, 179, 179);
        palette.setColor(QPalette::WindowText, prefixColor);
    } else {
        const QColor prefixColor = QColor(77, 77, 77);
        palette.setColor(QPalette::WindowText, prefixColor);
    }
    m_labformat->setPalette(palette);
}

void RenameDialog::slotsUpdate()
{
    setCurrentTip();
}
