// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printhelper.h"

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


#include "printhelper.h"
#include "unionimage/unionimage.h"



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

//暂时没有使用配置文件的快捷键，现在是根据代码中的快捷键
/*
static QAction *hookToolBarActionIcons(QToolBar *bar, QAction **pageSetupAction = nullptr)
{
    QAction *last_action = nullptr;

    for (QAction *action : bar->actions()) {
        const QString &text = action->text();

        if (text.isEmpty())
            continue;

        // 防止被lupdate扫描出来
        const char *context = "QPrintPreviewDialog";
        const char *print = "Print";

        const QMap<QString, QString> map {
            {QCoreApplication::translate(context, "Next page"), QStringLiteral("go-next")},
            {QCoreApplication::translate(context, "Previous page"), QStringLiteral("go-previous")},
            {QCoreApplication::translate(context, "First page"), QStringLiteral("go-first")},
            {QCoreApplication::translate(context, "Last page"), QStringLiteral("go-last")},
            {QCoreApplication::translate(context, "Fit width"), QStringLiteral("fit-width")},
            {QCoreApplication::translate(context, "Fit page"), QStringLiteral("fit-page")},
            {QCoreApplication::translate(context, "Zoom in"), QStringLiteral("zoom-in")},
            {QCoreApplication::translate(context, "Zoom out"), QStringLiteral("zoom-out")},
            {QCoreApplication::translate(context, "Portrait"), QStringLiteral("layout-portrait")},
            {QCoreApplication::translate(context, "Landscape"), QStringLiteral("layout-landscape")},
            {QCoreApplication::translate(context, "Show single page"), QStringLiteral("view-page-one")},
            {QCoreApplication::translate(context, "Show facing pages"), QStringLiteral("view-page-sided")},
            {QCoreApplication::translate(context, "Show overview of all pages"), QStringLiteral("view-page-multi")},
            {QCoreApplication::translate(context, print), QStringLiteral("print")},
            {QCoreApplication::translate(context, "Page setup"), QStringLiteral("page-setup")}
        };


        const QString &icon_name = map.value(action->text());

        if (icon_name.isEmpty())
            continue;

        if (pageSetupAction && icon_name == "page-setup") {
            *pageSetupAction = action;
        }

        QIcon icon(QStringLiteral(":/qt-project.org/dialogs/assets/images/qprintpreviewdialog/images/%1-24.svg").arg(icon_name));
//        action->setIcon(icon);
        last_action = action;
    }

    return last_action;
}
*/
void PrintHelper::showPrintDialog(const QStringList &paths, QWidget *parent)
{
    Q_UNUSED(parent)
    m_re->m_paths.clear();
    m_re->m_imgs.clear();

    m_re->m_paths = paths;
    QStringList tempExsitPaths;//保存存在的图片路径
    QImage imgTemp;
    for (const QString &path : paths) {
        QString errMsg;
        QImageReader imgReadreder(path);
        if (imgReadreder.imageCount() > 1) {
            for (int imgindex = 0; imgindex < imgReadreder.imageCount(); imgindex++) {
                imgReadreder.jumpToImage(imgindex);
                m_re->m_imgs << imgReadreder.read();
            }
        } else {
            //QImage不应该多次赋值，所以换到这里来，修复style问题
            QImage img;
            LibUnionImage_NameSpace::loadStaticImageFromFile(path, img, errMsg);
            if (!img.isNull()) {
                m_re->m_imgs << img;
            }
        }
        tempExsitPaths << paths;

    }
    //看图采用同步,因为只有一张图片
    DPrintPreviewDialog printDialog2(nullptr);
#if (DTK_VERSION_MAJOR > 5 \
    || (DTK_VERSION_MAJOR >=5 && DTK_VERSION_MINOR > 4) \
    || (DTK_VERSION_MAJOR >= 5 && DTK_VERSION_MINOR >= 4 && DTK_VERSION_PATCH >= 10))//5.4.4暂时没有合入
    //增加运行时版本判断
    if (DApplication::runtimeDtkVersion() >= DTK_VERSION_CHECK(5, 4, 10, 0)) {
        if (tempExsitPaths.count() > 0) {
            //直接传递为路径,不会有问题
            QString docName = QString(QFileInfo(tempExsitPaths.at(0)).completeBaseName());
            docName = docName + ".pdf";
            printDialog2.setDocName(docName);
        }
    }
#endif
    connect(&printDialog2, SIGNAL(paintRequested(DPrinter *)),
            m_re, SLOT(paintRequestSync(DPrinter *)));

#ifndef USE_TEST
    printDialog2.exec();
#else
    printDialog2.show();
#endif
    m_re->m_paths.clear();
    m_re->m_imgs.clear();
}

RequestedSlot::RequestedSlot(QObject *parent)
{
    Q_UNUSED(parent)
}

RequestedSlot::~RequestedSlot()
{

}

void RequestedSlot::paintRequestSync(DPrinter *_printer)
{
    //由于之前再度修改了打印的逻辑，导致了相同图片不在被显示，多余多页tiff来说不合理
    QPainter painter(_printer);
    int indexNum = 0;
    for (QImage img : m_imgs) {
        if (!img.isNull()) {
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            QRectF wRect = _printer->pageRect(QPrinter::DevicePixel);
            //修复bug98129，打印不完全问题，ratio应该是适应宽或者高，不应该直接适应宽
            qreal ratio = 0.0;
            qDebug() << wRect;
            ratio = wRect.width() * 1.0 / img.width();
            if (qreal(wRect.height() - img.height() * ratio) > 0) {
                painter.drawImage(QRectF(0, abs(qreal(wRect.height() - img.height() * ratio)) / 2,
                                         wRect.width(), img.height() * ratio), img);
            } else {
                ratio = wRect.height() * 1.0 / img.height();
                painter.drawImage(QRectF(qreal(wRect.width() - img.width() * ratio) / 2, 0,
                                         img.width() * ratio, wRect.height()), img);
            }


        }
        indexNum++;
        if (indexNum != m_imgs.size()) {
            _printer->newPage();

        }
    }
    painter.end();
}

