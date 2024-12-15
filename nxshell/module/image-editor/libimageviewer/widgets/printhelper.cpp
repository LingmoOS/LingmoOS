// SPDX-FileCopyrightText: 2020 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printhelper.h"
#include "service/permissionconfig.h"

#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPrinter>
#include <QPainter>
#include <QToolBar>
#include <QCoreApplication>
#include <QImageReader>
#include <QDebug>

#include <DApplication>
#include <dtkwidget_config.h>

#include "printhelper.h"
#include "unionimage/unionimage.h"

#include <QFileInfo>
PrintHelper *PrintHelper::m_Printer = nullptr;

PrintHelper *PrintHelper::getIntance()
{
    if (!m_Printer) {
        m_Printer = new PrintHelper();
    }
    return m_Printer;
}

PrintHelper::PrintHelper(QObject *parent)
    : QObject(parent)
{
    m_re = new RequestedSlot(this);
}

PrintHelper::~PrintHelper()
{
    m_re->deleteLater();
}

void PrintHelper::showPrintDialog(const QStringList &paths, QWidget *parent)
{
    if (!PermissionConfig::instance()->isPrintable()) {
        return;
    }

    m_re->clearPrintState();
    m_re->setPaths(paths);

    QStringList tempExsitPaths;  // 保存存在的图片路径
    for (const QString &path : paths) {
        QString errMsg;
        QImageReader imgReadreder(path);
        if (imgReadreder.imageCount() > 1) {
            for (int imgindex = 0; imgindex < imgReadreder.imageCount(); imgindex++) {
                imgReadreder.jumpToImage(imgindex);
                m_re->appendImage(imgReadreder.read());
            }
        } else {
            // QImage不应该多次赋值，所以换到这里来，修复style问题
            QImage img;
            LibUnionImage_NameSpace::loadStaticImageFromFile(path, img, errMsg);         
            if (!img.isNull()) {
                m_re->appendImage(img);
            }
        }
        tempExsitPaths << paths;
    }

    // 看图采用同步,因为只有一张图片，传入父指针
    DPrintPreviewDialog printDialog2(parent);
#if (DTK_VERSION_MAJOR > 5 || (DTK_VERSION_MAJOR >= 5 && DTK_VERSION_MINOR > 4) ||                                               \
     (DTK_VERSION_MAJOR >= 5 && DTK_VERSION_MINOR >= 4 && DTK_VERSION_PATCH >= 10))  // 5.4.4暂时没有合入
    // 增加运行时版本判断
    if (DApplication::runtimeDtkVersion() >= DTK_VERSION_CHECK(5, 4, 10, 0)) {
        if (!tempExsitPaths.isEmpty()) {
            // 直接传递为路径,不会有问题
            printDialog2.setDocName(QFileInfo(tempExsitPaths.at(0)).absoluteFilePath());
        }
    }
#endif

#ifdef DTKWIDGET_CLASS_DWaterMarkHelper
    // 检查是否过滤 DTK 部分设置
    if (PermissionConfig::instance()->installFilterPrintDialog(&printDialog2)) {
        qInfo() << qPrintable("Enable breakPrintSpacingLimit, filter print spacing config.");
    }

    // 定制分支，水印功能不依赖DTK版本，更新打印水印设置
    if (PermissionConfig::instance()->hasPrintWaterMark()) {
        PermissionConfig::instance()->setDialogPrintWatermark(&printDialog2);
    }
#endif

    connect(&printDialog2, SIGNAL(paintRequested(DPrinter *)), m_re, SLOT(paintRequestSync(DPrinter *)));

#ifndef USE_TEST
    int ret = printDialog2.exec();
#else
    printDialog2.show();
    int ret = QDialog::Accepted;
#endif

    // DTKWidget 在 5.6.9 版本前，无论是否打印，只会返回相同值 0 ，区分版本判断 (DTKWIDGET_CLASS_DWaterMarkHelper在 5.6.9 提供)
    // 没有直接以 DTKCore(DTK_VERSION_CHECK) 版本判断，DTKCore 和 DTKWidget 版本可能不同
#ifndef DTKWIDGET_CLASS_DWaterMarkHelper
    if (m_re->isPrinted()) {
#else
    if (QDialog::Accepted == ret) {
#endif
        if (!tempExsitPaths.isEmpty()) {
            PermissionConfig::instance()->triggerPrint(tempExsitPaths.first());
        }
    }

    m_re->clearPrintState();
}

RequestedSlot::RequestedSlot(QObject *parent)
{
    Q_UNUSED(parent)
}

RequestedSlot::~RequestedSlot() {}

void RequestedSlot::clearPrintState()
{
    m_paths.clear();
    m_imgs.clear();
    m_printed = false;
}

/**
   @brief 设置打印文件路径 `paths`
 */
void RequestedSlot::setPaths(const QStringList &paths)
{
    m_paths = paths;
}

/**
   @brief 追加打印图片数据 `img`
 */
void RequestedSlot::appendImage(const QImage &img)
{
    m_imgs << img;
}

/**
   @brief dtk打印组件请求绘制页面，将打印信息绘制到 `_printer` 中
 */
void RequestedSlot::paintRequestSync(DPrinter *_printer)
{
    // 由于之前再度修改了打印的逻辑，导致了相同图片不在被显示，多余多页tiff来说不合理
    QPainter painter(_printer);
    int indexNum = 0;
    for (QImage img : m_imgs) {
        if (!img.isNull()) {
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            QRect wRect = _printer->pageRect();
            // 修复bug98129，打印不完全问题，ratio应该是适应宽或者高，不应该直接适应宽
            qreal ratio = 0.0;
            qDebug() << wRect;
            ratio = wRect.width() * 1.0 / img.width();
            if (qreal(wRect.height() - img.height() * ratio) > 0) {
                painter.drawImage(
                    QRectF(0, abs(qreal(wRect.height() - img.height() * ratio)) / 2, wRect.width(), img.height() * ratio), img);
            } else {
                ratio = wRect.height() * 1.0 / img.height();
                painter.drawImage(QRectF(qreal(wRect.width() - img.width() * ratio) / 2, 0, img.width() * ratio, wRect.height()),
                                  img);
            }
        }
        indexNum++;
        if (indexNum != m_imgs.size()) {
            _printer->newPage();
        }
    }
    painter.end();

    if (!m_printed && 0 != indexNum) {
        m_printed = true;
    }
}
