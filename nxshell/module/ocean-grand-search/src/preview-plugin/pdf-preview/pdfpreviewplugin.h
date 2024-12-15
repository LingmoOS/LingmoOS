// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDFPREVIEWPLUGIN_H
#define PDFPREVIEWPLUGIN_H

#include "pdfpreview_global.h"

#include "previewplugin.h"

namespace GrandSearch {
namespace pdf_preview {

class PDFView;
class PDFPreviewPlugin : public QObject, public PreviewPlugin
{
    Q_OBJECT
public:
    explicit PDFPreviewPlugin(QObject *parent = nullptr);
    ~PDFPreviewPlugin() Q_DECL_OVERRIDE;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    bool previewItem(const ItemInfo &item) Q_DECL_OVERRIDE;
    ItemInfo item() const Q_DECL_OVERRIDE;
    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    bool stopPreview() Q_DECL_OVERRIDE;
    DetailInfoList getAttributeDetailInfo() const Q_DECL_OVERRIDE;
    QWidget *toolBarWidget() const Q_DECL_OVERRIDE;
    bool showToolBar() const Q_DECL_OVERRIDE;
private:
    ItemInfo m_item;
    PDFView *m_pdfView = nullptr;
};

}}

#endif // PDFPREVIEWPLUGIN_H
