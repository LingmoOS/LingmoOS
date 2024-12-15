// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uncompresspage.h"
#include "uncompressview.h"
#include "customwidget.h"
#include "popupdialog.h"
#include "DebugTimeManager.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "datamanager.h"

#include <DFontSizeManager>
#include <DFileDialog>

#include <QHBoxLayout>
#include <QShortcut>
#include <QFileInfo>
#include <QDebug>

UnCompressPage::UnCompressPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

UnCompressPage::~UnCompressPage()
{

}

void UnCompressPage::setArchiveFullPath(const QString &strArchiveFullPath, UnCompressParameter &unCompressPar)
{
    qInfo() << "加载压缩包：" << strArchiveFullPath;
    m_strArchiveFullPath = strArchiveFullPath;

    m_pUnCompressView->setArchivePath(m_strArchiveFullPath/*QFileInfo(m_strArchiveFullPath).path()*/);  // 设置压缩包路径
    if (UnCompressParameter::ST_No != unCompressPar.eSplitVolume) {
        // 若是分卷压缩包，不支持增/删/改
        m_pUnCompressView->setModifiable(false);
    } else {
        // 若不是分卷压缩包，按照支持的压缩类型，设置是否增/删/改，否则屏蔽这些操作
        m_pUnCompressView->setModifiable(unCompressPar.bModifiable, unCompressPar.bMultiplePassword);
    }
}

QString UnCompressPage::archiveFullPath()
{
    return m_strArchiveFullPath;
}

void UnCompressPage::setDefaultUncompressPath(const QString &strPath)
{
    m_strUnCompressPath = strPath;
    m_pUncompressPathBtn->setToolTip(m_strUnCompressPath);      // 设置解压路径提示信息
    m_pUncompressPathBtn->setText(elidedExtractPath(tr("Extract to:") + m_strUnCompressPath));  // 截取解压路径显示
    m_pUnCompressView->setDefaultUncompressPath(m_strUnCompressPath);
}

void UnCompressPage::refreshArchiveData()
{
    m_pUnCompressView->refreshArchiveData();
}

void UnCompressPage::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    m_pUncompressPathBtn->setText(elidedExtractPath(tr("Extract to:") + m_strUnCompressPath));
}

void UnCompressPage::refreshDataByCurrentPathChanged()
{
    m_pUnCompressView->refreshDataByCurrentPathChanged();
}

void UnCompressPage::addNewFiles(const QStringList &listFiles)
{
    m_pUnCompressView->addNewFiles(listFiles);
}

QString UnCompressPage::getCurPath()
{
    return m_pUnCompressView->getCurPath();
}

void UnCompressPage::clear()
{
    m_pUnCompressView->clear();
}

void UnCompressPage::initUI()
{
    m_strUnCompressPath = "~/Desktop";

    // 初始化相关变量
    m_pUnCompressView = new UnCompressView(this);
    m_pUncompressPathBtn = new CustomCommandLinkButton(tr("Extract to:") + " ~/Desktop", this);
    m_pUnCompressBtn = new CustomPushButton(tr("Extract", "button"), this);

    m_pUncompressPathBtn->setToolTip(m_strUnCompressPath);

    DFontSizeManager::instance()->bind(m_pUncompressPathBtn, DFontSizeManager::T8);

    // 解压路径布局
    QHBoxLayout *pPathLayout = new QHBoxLayout;
    pPathLayout->addStretch(1);
    pPathLayout->addWidget(m_pUncompressPathBtn, 2, Qt::AlignCenter);
    pPathLayout->addStretch(1);

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pUnCompressBtn, 2);
    pBtnLayout->addStretch(1);

    // 主界面布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(m_pUnCompressView);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pPathLayout);
    pMainLayout->addSpacing(10);
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setStretchFactor(m_pUnCompressView, 9);
    pMainLayout->setStretchFactor(pPathLayout, 1);
    pMainLayout->setStretchFactor(pBtnLayout, 1);
    pMainLayout->setContentsMargins(20, 1, 20, 20);


    // 设置快捷键
    auto openkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, &UnCompressPage::slotFileChoose);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void UnCompressPage::initConnections()
{
    connect(m_pUncompressPathBtn, &DPushButton::clicked, this, &UnCompressPage::slotUnCompressPathClicked);
    connect(m_pUnCompressBtn, &DPushButton::clicked, this, &UnCompressPage::slotUncompressClicked);
    connect(m_pUnCompressView, &UnCompressView::signalExtract2Path, this, &UnCompressPage::signalExtract2Path);
    connect(m_pUnCompressView, &UnCompressView::signalDelFiles, this, &UnCompressPage::signalDelFiles);
    connect(m_pUnCompressView, &UnCompressView::signalRenameFile, this, &UnCompressPage::signalRenameFile);
    connect(m_pUnCompressView, &UnCompressView::signalOpenFile, this, &UnCompressPage::signalOpenFile);
    connect(m_pUnCompressView, &UnCompressView::signalAddFiles2Archive, this, &UnCompressPage::signalAddFiles2Archive);
    connect(this, &UnCompressPage::sigRenameFile, m_pUnCompressView, &UnCompressView::sigRenameFile);
}

QString UnCompressPage::elidedExtractPath(const QString &strPath)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(strPath);//获取之前设置的字符串的像素大小
    QString pathStr = strPath;
    if (fontSize > width()) {
        pathStr = fontMetrics.elidedText(strPath, Qt::ElideMiddle, width());//返回一个带有省略号的字符串
    }

    return pathStr;
}

void UnCompressPage::slotUncompressClicked()
{
    QFileInfo file(m_strArchiveFullPath);
    PERF_PRINT_BEGIN("POINT-04", "压缩包名：" + file.fileName() + " 大小：" + QString::number(file.size()));

    // 判断解压路径是否有可执行权限或者路径是否存在进行解压创建文件
    QFileInfo m_fileDestinationPath(m_strUnCompressPath);
    bool m_permission = (m_fileDestinationPath.isWritable() && m_fileDestinationPath.isExecutable());

    if (!m_permission) { // 无法解压到已选中路径
        QString strDes;
        if (!m_fileDestinationPath.exists()) { // 路径不存在
            strDes = tr("The default extraction path does not exist, please retry");
        } else { // 路径无权限
            strDes = tr("You do not have permission to save files here, please change and retry");
        }

        TipDialog dialog(this);
        dialog.showDialog(strDes, tr("OK", "button"), DDialog::ButtonNormal);

        return;
    } else { // 发送解压信号
        emit signalUncompress(m_strUnCompressPath);
    }
}

void UnCompressPage::slotUnCompressPathClicked()
{
    // 创建文件选择对话框
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::Directory);
    dialog.setWindowTitle(tr("Find directory"));
    dialog.setDirectory(m_strUnCompressPath);

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    // 设置默认解压路径为选中的目录
    QList<QUrl> listUrl = dialog.selectedUrls();
    if (listUrl.count() > 0) {
        setDefaultUncompressPath(listUrl.at(0).toLocalFile());
    }
}

void UnCompressPage::slotFileChoose()
{
    if (m_pUnCompressView->isModifiable())
        emit signalFileChoose();
}

CustomCommandLinkButton *UnCompressPage::getUncompressPathBtn() const
{
    return m_pUncompressPathBtn;
}

CustomPushButton *UnCompressPage::getUnCompressBtn() const
{
    return m_pUnCompressBtn;
}

UnCompressView *UnCompressPage::getUnCompressView() const
{
    return m_pUnCompressView;
}
