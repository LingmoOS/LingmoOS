// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDFVIEW_H
#define PDFVIEW_H

#include "pdfpreview_global.h"

#include <QWidget>
#include <QFuture>

#include <dpdfdoc.h>

class QLabel;

namespace GrandSearch {
namespace pdf_preview {

class PDFView : public QWidget
{
    Q_OBJECT
public:
    static bool checkUosAiInstalled();

    explicit PDFView(const QString &file, QWidget *parent = nullptr);
    ~PDFView() Q_DECL_OVERRIDE;

    void initDoc(const QString &file);
    void initUI();
    void initConnections();
    QPixmap scaleAndRound(const QImage &img);
public slots:
    void onPageUpdated(QImage img);
    void showErrorPage();
signals:
    void pageUpdate(const QImage &img);
    void parseFailed();

private:
    void syncLoadFirstPage();

private:
    QLabel *m_pageLabel = nullptr;
    bool m_isBadDoc = false;
    bool m_isLoadFinished = false;
    QSharedPointer<DPdfDoc> m_doc = nullptr;
    QFuture<void> m_future;
    QImage m_pageImg;
};

}}

#endif // PDFVIEW_H
