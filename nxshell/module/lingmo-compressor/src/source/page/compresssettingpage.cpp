// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compresssettingpage.h"
#include "customwidget.h"
#include "pluginmanager.h"
#include "uitools.h"
#include "uistruct.h"
#include "popupdialog.h"
#include "DebugTimeManager.h"

#include <DApplication>
#include <DFileDialog>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QStandardPaths>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFormLayout>
#include <QFileIconProvider>
#include <QMenu>
#include <QMimeDatabase>
#include <QDebug>

#include <cmath>

#define SCROLL_MAX 621
#define SCROLL_MIN 348

TypeLabel::TypeLabel(QWidget *parent)
    : DFrame(parent)
{
    setFrameShape(QFrame::NoFrame);
}

TypeLabel::~TypeLabel()
{

}

void TypeLabel::mousePressEvent(QMouseEvent *event)
{
    emit labelClickEvent(event);
    DFrame::mousePressEvent(event);
}

void TypeLabel::paintEvent(QPaintEvent *event)
{
    if (hasFocus() && (m_reson & (Qt::TabFocusReason | Qt::BacktabFocusReason | Qt::PopupFocusReason))) {
        DStylePainter painter(this);
        DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        const QStyleOptionFrame *opt1 = &opt;
        DStyleHelper dstyle(style);
        int border_width = dstyle.pixelMetric(DStyle::PM_FocusBorderWidth, opt1, this);
        QColor color = dstyle.getColor(opt1, QPalette::Highlight);
        painter.setPen(QPen(color, border_width, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, opt1, &painter, this);
    }
    DFrame::paintEvent(event);
}

void TypeLabel::focusInEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    DFrame::focusInEvent(event);
}

void TypeLabel::focusOutEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    DFrame::focusOutEvent(event);
}

CompressSettingPage::CompressSettingPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
    slotAdvancedEnabled(m_pAdvancedBtn->isChecked());
}

CompressSettingPage::~CompressSettingPage()
{

}

void CompressSettingPage::setFileSize(const QStringList &listFiles, qint64 qSize)
{
    if (listFiles.count() == 0) {
        return;
    }

    m_listFiles = listFiles;
    m_qFileSize = qSize;

    QFileInfo fileinfobase(m_listFiles.at(0));

    QString strDefaultSavePath = fileinfobase.path();     // 初始化压缩包保存位置为第一个文件所在的位置

    // 若文件处于不同的位置，则压缩包保存位置默认为桌面
    for (int loop = 1; loop < m_listFiles.count(); loop++) {
        QFileInfo fileinfo(m_listFiles.at(loop));
        if (fileinfo.path() != fileinfobase.path()) {
            strDefaultSavePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            break;
        }
    }

    // 设置默认存储路径
    m_pSavePathEdt->setText(strDefaultSavePath);
    QUrl dir(strDefaultSavePath);
    m_pSavePathEdt->setDirectoryUrl(dir);

    // 设置默认压缩包名称
    if (1 == m_listFiles.count()) {       // 若是单文件
        if (fileinfobase.isDir()) {
            setDefaultName(fileinfobase.fileName());         // 如果是文件夹，压缩包名为文件夹
        } else {
            setDefaultName(fileinfobase.completeBaseName()); // 如果是文件，压缩包名为完整的文件名
        }
    } else {
        setDefaultName(tr("New Archive"));            // 如果是多文件，压缩包名为新建归档文件
    }
}

void CompressSettingPage::refreshMenu()
{
    m_pTypeMenu->clear();
    if (m_listSupportedMimeTypes.empty()) {
        m_listSupportedMimeTypes = PluginManager::get_instance().supportedWriteMimeTypes(PluginManager::SortByComment);     // 获取支持的压缩格式
    }

    QAction *pAction = nullptr;
    bool bHas7z = false;
    for (const QString &type : qAsConst(m_listSupportedMimeTypes)) {
        QMimeType mType = QMimeDatabase().mimeTypeForName(type);
        // QStringList suffixLst = mType.globPatterns(); 支持的后缀名
        QString suffix = mType.preferredSuffix();
        if(type.contains("x-7z-compressed")) {
            suffix = "7z";
        }
        if (suffix == "7z") {
            bHas7z = true;
        }

        pAction = new QAction(suffix, m_pTypeMenu);
        pAction->setData(type);
        m_pTypeMenu->addAction(pAction);

    }

    // 判断是否安装了7z，若安装了7z，则压缩类型添加tar.7z
    if (bHas7z) {
        // 判断是否已经有压缩类型，tzr.7z添加在zip前面
        if (pAction != nullptr) {
            m_pTypeMenu->insertAction(pAction, new QAction("tar.7z", m_pTypeMenu));
        } else {
            m_pTypeMenu->addAction("tar.7z");
        }
    }

    // 默认选择类型为zip
    if (pAction != nullptr) {
        slotTypeChanged(pAction);
    }
}

void CompressSettingPage::initUI()
{
    // 左侧界面
    m_pTypePixmapLbl = new DLabel(this);
    m_pClickLbl = new TypeLabel(this);
    m_pCompressTypeLbl = new DLabel(this);
    pArrowPixmapLbl = new DLabel(this);

    m_pClickLbl->setMinimumWidth(125);
    m_pClickLbl->setObjectName("ClickTypeLabel");
    m_pClickLbl->setFocusPolicy(Qt::TabFocus);
    m_pClickLbl->installEventFilter(this);

    DStyle style;   // 设置菜单箭头
    QPixmap pixmap = style.standardIcon(DStyle::StandardPixmap::SP_ReduceElement).pixmap(QSize(10, 10));
    pArrowPixmapLbl->setPixmap(pixmap);

    DFontSizeManager::instance()->bind(m_pCompressTypeLbl, DFontSizeManager::T5, QFont::DemiBold);

    // 右侧界面
    DLabel *pAdvancedLbl = new DLabel(tr("Advanced Options"), this);
    pAdvancedLbl->setForegroundRole(DPalette::WindowText);
    m_pFileNameEdt = new DLineEdit(this);
    m_pSavePathEdt = new DFileChooserEdit(this);
    m_pCompressLevelLbl = new DLabel(tr("Compression method") + ":", this);
    m_pCompressLevelCmb = new CustomCombobox(this);
    m_pAdvancedBtn = new CustomSwitchButton(this);
    m_pEncryptedLbl = new DLabel(tr("Encrypt the archive") + ":", this);
    m_pPasswordEdt = new DPasswordEdit(this);
    m_pCpuLbl = new DLabel(tr("CPU threads") + ":", this);
    m_pCpuCmb = new CustomCombobox(this);
    m_pListEncryptionLbl = new DLabel(tr("Encrypt the file list too"), this);
    m_pListEncryptionBtn = new CustomSwitchButton(this);
    m_pSplitCkb = new CustomCheckBox(tr("Split to volumes") + ":", this);
    m_pSplitValueEdt = new DDoubleSpinBox(this);
    m_pCommentLbl = new DLabel(tr("Comment") + ":", this);
    m_pCommentEdt = new DTextEdit(this);
    m_pCompressBtn = new CustomPushButton(tr("Compress", "button"), this);

    m_pFileNameEdt->setMinimumWidth(260);    // 配置文件名属性
    QLineEdit *pNameEdt = m_pFileNameEdt->lineEdit();
    pNameEdt->setMaxLength(70);
    m_pFileNameEdt->setText(tr("New Archive"));

    m_pSavePathEdt->setFileMode(DFileDialog::Directory);        // 配置保存路径
    m_pSavePathEdt->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    m_pSavePathEdt->setMinimumWidth(260);

    m_pCompressLevelCmb->setMinimumWidth(260);   // 设置压缩方式尺寸
    m_pCompressLevelCmb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 跟随界面缩放


    m_pCpuCmb->setMinimumWidth(260);   // 设置压缩方式尺寸
    m_pCpuCmb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 跟随界面缩放

    // 设置压缩方式选项
    QStringList listCompressLevel = QStringList() << tr("Store") << tr("Fastest") << tr("Fast") << tr("Normal") << tr("Good") << tr("Best");
    // 添加压缩方式
    m_pCompressLevelCmb->addItems(listCompressLevel);

    m_pCompressLevelCmb->setCurrentIndex(2);

    // 设置CPU线程数
    QStringList listCpuThread = QStringList() << tr("Single thread") << tr("2 threads") << tr("4 threads") << tr("8 threads");
    // 添加压缩方式
    m_pCpuCmb->addItems(listCpuThread);

    m_pCpuCmb->setCurrentIndex(0);

    pAdvancedLbl->setForegroundRole(DPalette::WindowText);

    m_pPasswordEdt->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);   // 密码框默认屏蔽输入法

    m_pEncryptedLbl->setToolTip(tr("Support zip, 7z type only"));   // 设置加密文件提示语
    m_pEncryptedLbl->setForegroundRole(DPalette::WindowText);

    m_pListEncryptionLbl->setToolTip(tr("Support 7z type only"));   // 设置列表加密提示语
    m_pListEncryptionLbl->setEnabled(false);

    // 待确认
    m_pSplitValueEdt->setMinimumWidth(260);
    m_pSplitValueEdt->setSuffix("MB");
    m_pSplitValueEdt->setRange(0.0, 1000000);
    m_pSplitValueEdt->setSingleStep(0.1);
    m_pSplitValueEdt->setDecimals(1);
    m_pSplitValueEdt->setValue(0.0);
    m_pSplitValueEdt->setSpecialValueText(" ");

    m_pCommentEdt->setPlaceholderText(tr("Enter up to %1 characters").arg(MAXCOMMENTLEN));
    m_pCommentEdt->setTabChangesFocus(true); // DTextEdit中Tab键切换焦点

    m_pCompressBtn->setMinimumWidth(340);    // 设置压缩按钮最小尺寸

    // 左侧布局
    QHBoxLayout *pTypeLayout = new QHBoxLayout; // 类型布局
    pTypeLayout->addStretch();
    pTypeLayout->addWidget(m_pCompressTypeLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pTypeLayout->addWidget(pArrowPixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pTypeLayout->addStretch();
    m_pClickLbl->setLayout(pTypeLayout);

    QVBoxLayout *pLeftLayout = new QVBoxLayout; // 左侧整体布局
    pLeftLayout->addStretch();      // task 16309调整最小大小
    pLeftLayout->addWidget(m_pTypePixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pLeftLayout->addWidget(m_pClickLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pLeftLayout->addStretch();

    // 右侧布局
    DLabel *pLabName = new DLabel(tr("Name") + ":", this);
    DLabel *pLabSave = new DLabel(tr("Save to") + ":", this);

    QHBoxLayout *pAdvancedLayout = new QHBoxLayout();       // 高级选项布局
    pAdvancedLayout->addWidget(pAdvancedLbl, 0, Qt::AlignLeft);
    pAdvancedLayout->addWidget(m_pAdvancedBtn, 1, Qt::AlignRight);

    QHBoxLayout *pListEncryptionLayout = new QHBoxLayout();    // 列表加密布局
    pListEncryptionLayout->addWidget(m_pListEncryptionLbl, 0, Qt::AlignLeft);
    pListEncryptionLayout->addWidget(m_pListEncryptionBtn, 1, Qt::AlignRight);

    // 右侧内容布局
    QVBoxLayout *pRightLayout = new QVBoxLayout;
    pRightLayout->setSpacing(5);
    pRightLayout->addWidget(pLabName);
    pRightLayout->addWidget(m_pFileNameEdt);
    pRightLayout->addSpacing(7);
    pRightLayout->addWidget(pLabSave);
    pRightLayout->addWidget(m_pSavePathEdt);
    pRightLayout->addWidget(m_pCompressLevelLbl);
    pRightLayout->addWidget(m_pCompressLevelCmb);
    pRightLayout->addLayout(pAdvancedLayout);
    pRightLayout->addWidget(m_pCpuLbl);
    pRightLayout->addWidget(m_pCpuCmb);
    pRightLayout->addWidget(m_pEncryptedLbl);
    pRightLayout->addWidget(m_pPasswordEdt);
    pRightLayout->addLayout(pListEncryptionLayout);
    pRightLayout->addWidget(m_pSplitCkb);
    pRightLayout->addWidget(m_pSplitValueEdt);
    pRightLayout->addWidget(m_pCommentLbl);
    pRightLayout->addWidget(m_pCommentEdt);
    pRightLayout->addStretch();
    pRightLayout->setContentsMargins(0, 0, 50, 0);

    // 右侧滚动区域
    m_pRightScroll = new QScrollArea(this);
    DWidget *pRightWgt = new DWidget(this);
    pRightWgt->setLayout(pRightLayout);
    m_pRightScroll->setFrameShape(QFrame::NoFrame);
    m_pRightScroll->setWidgetResizable(true);
    m_pRightScroll->setMinimumHeight(100);           // task 16309调整最小大小
    m_pRightScroll->setWidget(pRightWgt);
    m_pRightScroll->setFocusPolicy(Qt::NoFocus);
    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pCompressBtn, 2);
    pBtnLayout->addStretch(1);

    // 主布局
    QHBoxLayout *pContentLayout = new QHBoxLayout;  // 内容布局
    pContentLayout->addStretch();
    pContentLayout->addLayout(pLeftLayout, 8);
    pContentLayout->addStretch();
    pContentLayout->addWidget(m_pRightScroll, 10);
    pContentLayout->addStretch();
    pContentLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);   // 主布局
    pMainLayout->addLayout(pContentLayout, 10);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setContentsMargins(0, 6, 0, 20);

    // 初始化菜单
    m_pTypeMenu = new QMenu(this);
    m_pTypeMenu->setMinimumWidth(162);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);

    // bug103712  滚动区域内widget高度发生变化导致页面闪动
    pRightWgt ->setFixedHeight(pRightWgt->height());
}

void CompressSettingPage::initConnections()
{
    connect(m_pClickLbl, SIGNAL(labelClickEvent(QMouseEvent *)), this, SLOT(slotShowRightMenu(QMouseEvent *)));
    connect(m_pTypeMenu, &DMenu::triggered, this, &CompressSettingPage::slotTypeChanged);
    connect(m_pFileNameEdt, &DLineEdit::textChanged, this, &CompressSettingPage::slotRefreshFileNameEdit);
    connect(m_pAdvancedBtn, &DSwitchButton::toggled, this, &CompressSettingPage::slotAdvancedEnabled);
    connect(m_pSplitCkb, &DCheckBox::stateChanged, this, &CompressSettingPage::slotSplitEdtEnabled);
    connect(m_pCompressBtn, &DPushButton::clicked, this, &CompressSettingPage::slotCompressClicked);
    connect(m_pPasswordEdt, &DPasswordEdit::echoModeChanged, this, &CompressSettingPage::slotEchoModeChanged);
    connect(m_pPasswordEdt, &DPasswordEdit::textEdited, this, &CompressSettingPage::slotPasswordChanged);
    connect(m_pCommentEdt, &DTextEdit::textChanged, this, &CompressSettingPage::slotCommentTextChanged);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &CompressSettingPage::slotRefreshFileNameEdit);
}

void CompressSettingPage::setTypeImage(const QString &strType)
{
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo("temp." + strType));

    m_pTypePixmapLbl->setPixmap(icon.pixmap(128, 128));
}

bool CompressSettingPage::checkFileNameVaild(const QString &strText)
{
    QString strArchiveName = strText;
    strArchiveName = strArchiveName.remove(" ");

    // 文件名为空返回错误
    if (strArchiveName.length() == 0) {
        return false;
    }

    // 文件名过长返回错误
    if (strArchiveName.length() > 255) {
        return false;
    }

    // 如果文件名中包含"/"，返回错误
    if (strArchiveName.contains(QDir::separator())) {
        return false;
    }

    return true;
}

void CompressSettingPage::setEncryptedEnabled(bool bEnabled)
{
    // 设置加密是否可用
    m_pEncryptedLbl->setEnabled(bEnabled);
    m_pPasswordEdt->setEnabled(bEnabled);

    if (!bEnabled) {
        setListEncryptionEnabled(bEnabled);
    }
}

void CompressSettingPage::slotEchoModeChanged(bool bEchoOn)
{
    // 根据明暗码决定是否屏蔽输入法
    m_pPasswordEdt->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, bEchoOn);
}

/**
 * @brief setListEncryptionEnabled  设置列表加密是否可用
 * @param bEnabled  是否可用
 */
void CompressSettingPage::setListEncryptionEnabled(bool bEnabled)
{
    m_pListEncryptionLbl->setEnabled(bEnabled);
    m_pListEncryptionBtn->setEnabled(bEnabled);
}

void CompressSettingPage::setSplitEnabled(bool bEnabled)
{
    m_pSplitCkb->setEnabled(bEnabled);  // 设置分卷勾选框是否可用

    // 如果不分卷，取消勾选，清空分卷数据
    if (!bEnabled) {
        m_pSplitCkb->setCheckState(Qt::Unchecked);
        m_pSplitValueEdt->setEnabled(bEnabled);
        m_pSplitValueEdt->clear();
        m_pSplitValueEdt->setValue(0.0);
    } else {
        m_pSplitValueEdt->setEnabled(m_pSplitCkb->isChecked());
    }
}

void CompressSettingPage::refreshCompressLevel(const QString &strType)
{
    if (0 == strType.compare("tar")) {
        // tar只有存储功能
        m_pCompressLevelCmb->setCurrentIndex(0);
        // 设置压缩方式不可用
        m_pCompressLevelCmb->setEnabled(false);
        m_pCompressLevelLbl->setEnabled(false);
    } else if (0 == strType.compare("tar.Z")) {
        // tar.Z无压缩方式，使用默认，即标准
        m_pCompressLevelCmb->setCurrentIndex(3);
        // 设置压缩方式不可用
        m_pCompressLevelCmb->setEnabled(false);
        m_pCompressLevelLbl->setEnabled(false);
    } else {
        // 其余格式支持设置压缩方式
        // 设置压缩方式可用
        m_pCompressLevelCmb->setEnabled(true);
        m_pCompressLevelLbl->setEnabled(true);
    }
}

void CompressSettingPage::setCommentEnabled(bool bEnabled)
{
    m_pCommentLbl->setEnabled(bEnabled);
    m_pCommentEdt->setEnabled(bEnabled);
    // 注释不可用时,清除注释
    if (!bEnabled)
        m_pCommentEdt->clear();
}

bool CompressSettingPage::checkCompressOptionValid()
{
    if (!checkFileNameVaild(m_pFileNameEdt->text())) { // 检查文件名是否有效
        showWarningDialog(tr("Invalid file name"));
        return false;
    }

    QFileInfo fInfo(m_pSavePathEdt->text());
    if ((m_pSavePathEdt->text().remove(" ")).isEmpty()) { // 检查是否已经选择保存路径
        showWarningDialog(tr("Please enter the path"));
        return false;
    }

    if (!fInfo.exists()) { // 检查选择保存路径是否存在
        showWarningDialog(tr("The path does not exist, please retry"));
        return false;
    }

    if (!(fInfo.isWritable() && fInfo.isExecutable())) { // 检查一选择保存路径是否有权限
        showWarningDialog(tr("You do not have permission to save files here, please change and retry"));
        return false;
    }

    if (m_pSplitCkb->isChecked()
            && (fabs(m_pSplitValueEdt->value()) < std::numeric_limits<double>::epsilon()
                || (((m_qFileSize / 1024 / 1024) / (m_pSplitValueEdt->value())) > 200))
            && (m_strMimeType.contains("7z") || m_strMimeType.contains("zip"))) { // 最多允许分卷数量为200卷
        showWarningDialog(tr("Too many volumes, please change and retry"));
        return false;
    }

    bool bResult = false;
    for (int i = 0; i < m_listFiles.count(); i++) {
        QFileInfo file(m_listFiles.at(i));
        if (!file.exists()) {  // 待压缩文件已经不存在
            bResult = false;
            showWarningDialog(tr("%1 does not exist on the disk, please check and try again").arg(file.absoluteFilePath()), file.absoluteFilePath());
            return false;
        }

        if (!file.isReadable()) {
            bResult = false;
            showWarningDialog(tr("You do not have permission to compress %1").arg(file.absoluteFilePath()), file.absoluteFilePath());
            return false;
        }

        if (file.isDir()) { // 检查文件夹和文件夹下子文件是否可读
            bResult = checkFile(file.absoluteFilePath());
            if (!bResult)
                return bResult;
        }
    }

    return true;
}

bool CompressSettingPage::checkFile(const QString &path)
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists()) {  // 待压缩文件已经不存在
        if (fileInfo.isSymLink()) {
            showWarningDialog(tr("The original file of %1 does not exist, please check and try again").arg(fileInfo.absoluteFilePath()), fileInfo.absoluteFilePath());
        } else {
            showWarningDialog(tr("%1 does not exist on the disk, please check and try again").arg(fileInfo.absoluteFilePath()), fileInfo.absoluteFilePath());
        }

        return false;
    }

    if (!fileInfo.isReadable()) {
        showWarningDialog(tr("You do not have permission to compress %1").arg(fileInfo.absoluteFilePath()), fileInfo.absoluteFilePath());
        return false;
    }

    if (fileInfo.isDir()) {
        QDir dir(path);
        // 遍历文件夹下的子文件夹
        QFileInfoList listInfo = dir.entryInfoList(QDir::AllEntries | QDir::System
                                                   | QDir::NoDotAndDotDot | QDir::Hidden);

        foreach (QFileInfo subInfo, listInfo) {
            bool bResult = checkFile(subInfo.absoluteFilePath());
            if (!bResult)
                return bResult;
        }


    }

    return true;
}

int CompressSettingPage::showWarningDialog(const QString &msg, const QString &strToolTip)
{
    // 使用封装好的提示对话框
    TipDialog dialog(this);
    return dialog.showDialog(msg, tr("OK", "button"), DDialog::ButtonNormal, strToolTip);
}

void CompressSettingPage::setDefaultName(const QString &strName)
{
    m_pFileNameEdt->setText(strName);
    QLineEdit *qfilename = m_pFileNameEdt->lineEdit();
    qfilename->selectAll();
    qfilename->setFocus();
}

void CompressSettingPage::slotShowRightMenu(QMouseEvent *e)
{
    Q_UNUSED(e)

    // 设置菜单弹出位置
    m_pTypeMenu->popup(m_pCompressTypeLbl->mapToGlobal(m_pCompressTypeLbl->pos()));
}

void CompressSettingPage::slotTypeChanged(QAction *action)
{
    if (nullptr == action)
        return;

    m_strMimeType = action->data().toString();

    QString selectType = action->text();
    setTypeImage(selectType);
    m_pCompressTypeLbl->setText(selectType);

    m_pCpuLbl->setEnabled(false);
    m_pCpuCmb->setEnabled(false);

    if (0 == selectType.compare("tar.7z")) {       // tar.7z支持普通/列表加密，不支持分卷
        setEncryptedEnabled(true);
        setListEncryptionEnabled(true);
        setSplitEnabled(false);
        setCommentEnabled(false);
    } else if (0 == selectType.compare("7z")) {     // 7z支持普通/列表加密。支持分卷
        setEncryptedEnabled(true);
        setListEncryptionEnabled(true);
        setSplitEnabled(true);
        setCommentEnabled(false);
    } else if (0 == selectType.compare("zip")) {    // zip支持普通加密，不支持列表加密，支持分卷，支持注释，但不同时支持分卷和注释
        setEncryptedEnabled(true);
        setListEncryptionEnabled(false);
        setSplitEnabled(true);
        setCommentEnabled(!m_pSplitCkb->isChecked()); // 不同时支持分卷和注释
    } else {                                // 其余格式不支持加密，不支持分卷
        setEncryptedEnabled(false);
        setListEncryptionEnabled(false);
        setSplitEnabled(false);
        setCommentEnabled(false);

        if (0 == selectType.compare("tar.gz")) {
            m_pCpuLbl->setEnabled(true);
            m_pCpuCmb->setEnabled(true);
        }
    }

    refreshCompressLevel(selectType);
}

void CompressSettingPage::slotRefreshFileNameEdit()
{
    DPalette plt = DApplicationHelper::instance()->palette(m_pFileNameEdt);

    if (!m_pFileNameEdt->text().isEmpty()) {
        // 检测文件名合法性
        if (false == checkFileNameVaild(m_pFileNameEdt->text())) {
            plt.setBrush(DPalette::Text, plt.color(DPalette::TextWarning));     // 警告色
        } else {
            plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));      // 正常颜色
        }
    } else {
        plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));
    }

    m_pFileNameEdt->setPalette(plt);
}

void CompressSettingPage::slotAdvancedEnabled(bool bEnabled)
{
    // bug103712  滚动区域内widget高度发生变化导致页面闪动   页面变化前先设置widget大小
    if (bEnabled) {
        m_pRightScroll->widget()->setFixedHeight(SCROLL_MAX);
    } else {
        m_pRightScroll->widget()->setFixedHeight(SCROLL_MIN);
    }
    // 设置控件是否隐藏
    m_pEncryptedLbl->setVisible(bEnabled);
    m_pPasswordEdt->setVisible(bEnabled);
    m_pListEncryptionLbl->setVisible(bEnabled);
    m_pListEncryptionBtn->setVisible(bEnabled);
    m_pSplitCkb->setVisible(bEnabled);
    m_pSplitValueEdt->setVisible(bEnabled);
    m_pCommentLbl->setVisible(bEnabled);
    m_pCommentEdt->setVisible(bEnabled);
    m_pCpuLbl->setVisible(bEnabled);
    m_pCpuCmb->setVisible(bEnabled);

    // 不启用高级选项时清空界面数据
    if (!bEnabled) {
        m_pPasswordEdt->clear();
        m_pListEncryptionBtn->setChecked(false);
        m_pSplitCkb->setChecked(false);
        m_pSplitValueEdt->clear();
        m_pSplitValueEdt->setValue(0.0);
        m_pCpuCmb->setCurrentIndex(0);
        m_pCommentEdt->clear();
    }


}

void CompressSettingPage::slotSplitEdtEnabled()
{
    // 设置分卷输入框是否可用
    m_pSplitValueEdt->setEnabled(m_pSplitCkb->isChecked());

    if (m_pSplitCkb->isChecked()) {     // 显示单位大小
        QString size = UiTools::humanReadableSize(m_qFileSize, 1);
        m_pSplitValueEdt->setToolTip(tr("Total size: %1").arg(size));
    } else {
        m_pSplitValueEdt->clear();
        m_pSplitValueEdt->setValue(0.0);
    }

    if (m_strMimeType.contains("zip")) {
        setCommentEnabled(!m_pSplitCkb->isChecked()); // 不能同时支持zip分卷和注释
    }
}

void CompressSettingPage::slotCompressClicked()
{
    QString strTmpCompresstype = m_pCompressTypeLbl->text();        // 压缩格式
    QString strName = m_pFileNameEdt->text() + "." + strTmpCompresstype;   // 压缩包名称
    PERF_PRINT_BEGIN("POINT-03", "压缩包名：" + strName + " 大小：" + QString::number(m_qFileSize));
    qInfo() << "点击了压缩按钮";

    // 检查源文件中是否包含即将生成的压缩包
    if (m_listFiles.contains(m_pSavePathEdt->text() + QDir::separator() + strName)) {
        showWarningDialog(tr("The name is the same as that of the compressed archive, please use another one"));
        return;
    }

    //压缩zip分卷不支持中文密码
    if (m_pSplitCkb->isChecked()
            && "zip" == m_pCompressTypeLbl->text()
            && (!m_pPasswordEdt->text().isEmpty() && m_pPasswordEdt->text().contains(QRegExp("[\\x4e00-\\x9fa5]+")))) {
        m_pPasswordEdt->showAlertMessage(tr("The password for ZIP volumes cannot be in Chinese"));
        return;
    }

    // 检查合法性
    if (!checkCompressOptionValid()) {
        return;
    }

    CompressParameter compressInfo;

    compressInfo.strMimeType = m_strMimeType;       // 格式类型
    compressInfo.strArchiveName = strName;   // 压缩包名称
    compressInfo.strTargetPath = m_pSavePathEdt->text();    // 压缩包保存路径
    compressInfo.strPassword = m_pPasswordEdt->lineEdit()->text();
    compressInfo.bEncryption = !(compressInfo.strPassword.isEmpty());     // 是否加密
    compressInfo.strEncryptionMethod = compressInfo.bEncryption ? "AES256" : "";       // 加密算法
    compressInfo.bHeaderEncryption = m_pListEncryptionBtn->isChecked();     // 是否列表加密
    compressInfo.bSplit = m_pSplitCkb->isChecked();     // 是否分卷
    compressInfo.iVolumeSize = static_cast< int >(m_pSplitValueEdt->value() * 1024);    // 分卷大小
    compressInfo.bTar_7z = ("tar.7z" == strTmpCompresstype) ? true : false;     // 是否为tar.7z格式
    compressInfo.qSize = m_qFileSize;

    // 线程数
    switch (m_pCpuCmb->currentIndex()) {
    case 0:
        compressInfo.iCPUTheadNum = 1;
        break;
    case 1:
        compressInfo.iCPUTheadNum = 2;
        break;
    case 2:
        compressInfo.iCPUTheadNum = 4;
        break;
    case 3:
        compressInfo.iCPUTheadNum = 8;
        break;
    default:
        compressInfo.iCPUTheadNum = 1;
        break;
    }

    // 压缩等级
    // tar、tar.Z:使用默认压缩方式
    // bz2、lzo 1-9:取1、3、5、6、7、9
    // 其它 0-9:取0、1、3、5、7、9
    QList<int> listLevel;
    int iLevel = 0;
    if (0 == strTmpCompresstype.compare("tar") || 0 == strTmpCompresstype.compare("tar.Z")) {
        iLevel = -1;    // -1为不设置压缩方式，使用默认压缩方式
    } else if (0 == strTmpCompresstype.compare("tar.bz2") || 0 == strTmpCompresstype.compare("tar.lzo")) {
        listLevel << 1 << 3 << 5 << 6 << 7 << 9;    // 为tar.bz2、tar.lzo设置6种压缩方式
        iLevel = listLevel[m_pCompressLevelCmb->currentIndex()];
    } else {
        listLevel << 0 << 1 << 3 << 5 << 7 << 9;    // 其余格式设置6种压缩方式
        iLevel = listLevel[m_pCompressLevelCmb->currentIndex()];
    }

    compressInfo.iCompressionLevel = iLevel;

    // 检测此压缩包名称是否存在
    QFileInfo fileInfo(compressInfo.strTargetPath + QDir::separator() + compressInfo.strArchiveName);
    if (fileInfo.exists()) {
        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(tr("Another file with the same name already exists, replace it?"), tr("Cancel", "button"), DDialog::ButtonNormal, tr("Replace", "button"), DDialog::ButtonWarning);
        if (1 == iResult) {     // 如果点击替换，先移除本地压缩包
            QFile file(fileInfo.filePath());
            file.remove();
        } else {    // 点击关闭或者取消，不操作
            return;
        }
    }

    emit signalCompressClicked(QVariant::fromValue(compressInfo));
}

void CompressSettingPage::slotCommentTextChanged()
{
    QString savetext = m_pCommentEdt->toPlainText();
    if (savetext.size() > MAXCOMMENTLEN) { //限制最多注释MAXCOMMENTLEN个字
        // 保留前MAXCOMMENTLEN个注释字符
        m_pCommentEdt->setText(savetext.left(MAXCOMMENTLEN));

        //设定鼠标位置，将鼠标放到最后的地方
        QTextCursor cursor = m_pCommentEdt->textCursor();
        cursor.setPosition(MAXCOMMENTLEN);
        m_pCommentEdt->setTextCursor(cursor);
    }
}

void CompressSettingPage::slotPasswordChanged()
{
    QRegExp reg("^[\\x00-\\x80\\x4e00-\\x9fa5]+$");
    QString pwdin = m_pPasswordEdt->lineEdit()->text();
    QString pwdout;

    if (!pwdin.isEmpty() && !reg.exactMatch(pwdin)) {
        for (int i = 0; i < pwdin.length(); ++i) {
            if (reg.exactMatch(pwdin.at(i))) {
                pwdout.push_back(pwdin.at(i));
            }
        }
        m_pPasswordEdt->setText(pwdout);
        // 仅支持中英文字符及部分符号
        m_pPasswordEdt->showAlertMessage(tr("Only Chinese and English characters and some symbols are supported"));
    }
}

CustomPushButton *CompressSettingPage::getCompressBtn() const
{
    return m_pCompressBtn;
}

QString CompressSettingPage::getComment() const
{
    // m_pCommentEdt不可用时就返回空字符串，表示该压缩格式(除zip格式外)不支持添加注释
    if (nullptr != m_pCommentEdt) {
        return m_pCommentEdt->isEnabled() ? m_pCommentEdt->toPlainText() : QString("");
    } else {
        return QString("");
    }
}

bool CompressSettingPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pClickLbl) {
        if (QEvent::KeyPress == event->type()) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (Qt::Key_Enter == keyEvent->key() || Qt::Key_Return == keyEvent->key()) { //响应"回车键"
                m_pTypeMenu->popup(m_pCompressTypeLbl->mapToGlobal(m_pCompressTypeLbl->pos())); // 设置菜单弹出位置
                m_pTypeMenu->setFocus(); // 切换焦点
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        // pass the event on to the parent class
        return DWidget::eventFilter(watched, event);
    }
}

TypeLabel *CompressSettingPage::getClickLbl() const
{
    return m_pClickLbl;
}
