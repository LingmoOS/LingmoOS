// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreview_global.h"
#include "pdfview.h"
#include "global/commontools.h"

#include <dpdfpage.h>

#include <QDebug>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QPainter>
#include <QLabel>
#include <QPainterPath>
#include <QApplication>
#include <QScreen>

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::pdf_preview;

#define PAGE_FIXED_SIZE   QSize(360, 386)

PDFView::PDFView(const QString &file, QWidget *parent)
    : QWidget(parent)
{
    initDoc(file);
    initUI();
    initConnections();
}

PDFView::~PDFView()
{
    if (!m_isLoadFinished)
        m_future.waitForFinished();
}

void PDFView::initDoc(const QString &file)
{
    m_doc.reset(new DPdfDoc(file));
    if (!m_doc || m_doc->status() != DPdfDoc::SUCCESS) {
        qWarning() << "Cannot read this pdf file: " << file;
        m_isBadDoc = true;
    }
}

void PDFView::initUI()
{
    this->setFixedSize(PAGE_FIXED_SIZE);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_pageLabel = new QLabel(this);
    // 居中显示
    layout->addStretch();
    layout->addWidget(m_pageLabel);
    layout->addStretch();

    if (m_isBadDoc) {
        showErrorPage();
        return;
    }
    syncLoadFirstPage();
}

void PDFView::initConnections()
{
    connect(this, &PDFView::pageUpdate, this, &PDFView::onPageUpdated);
    connect(this, &PDFView::parseFailed, this, &PDFView::showErrorPage);
}

QPixmap PDFView::scaleAndRound(const QImage &img)
{
    auto pixmap = QPixmap::fromImage(img);
    // 缩放
    pixmap = pixmap.scaledToWidth(m_pageLabel->width(), Qt::SmoothTransformation);

    QPixmap destImage(m_pageLabel->width(), std::min(pixmap.height(), PAGE_FIXED_SIZE.height()));
    destImage.fill(Qt::transparent);
    QPainter painter(&destImage);
    // 抗锯齿
    painter.setRenderHints(QPainter::Antialiasing, true);
    // 图片平滑处理
    painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
    // 将图片裁剪为圆角
    QPainterPath path;
    QRect rect(0, 0, destImage.width(), destImage.height());
    path.addRoundedRect(rect, 8, 8);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, destImage.width(), destImage.height(), pixmap);

    return destImage;
}

void PDFView::showErrorPage()
{
    m_pageLabel->setFixedSize(PAGE_FIXED_SIZE);
    QImage errImg(":/icons/file_damaged.svg");
    errImg = errImg.scaled(70, 70);
    errImg = CommonTools::creatErrorImage(PAGE_FIXED_SIZE, errImg);

    auto errPixmap = scaleAndRound(errImg);
    m_pageLabel->setPixmap(errPixmap);
}

void PDFView::onPageUpdated(QImage img)
{
    auto pixmap = scaleAndRound(img);
    m_pageLabel->setPixmap(pixmap);
}

void PDFView::syncLoadFirstPage()
{
    m_future = QtConcurrent::run([=] {
        if (m_doc->pageCount() > 0) {
            qreal docXRes = 72;
            qreal docYRes = 72;
            QScreen *srn = QApplication::screens().value(0);
            if (nullptr != srn) {
                docXRes = srn->physicalDotsPerInchX();
                docYRes = srn->physicalDotsPerInchY();
            }

            DPdfPage *page = m_doc->page(0, docXRes, docYRes);
            if (!page || !page->isValid()) {
                emit parseFailed();
                m_isLoadFinished = true;
                return;
            }

            QSizeF size = page->sizeF();
            QImage img = page->image(static_cast<int>(size.width()), static_cast<int>(size.height()));

            emit pageUpdate(img);
        }

        m_isLoadFinished = true;
    });
}
