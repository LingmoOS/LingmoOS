// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreview_global.h"
#include "pdfpreviewplugin.h"
#include "pdfview.h"

#include <QFileInfo>

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::pdf_preview;

PDFPreviewPlugin::PDFPreviewPlugin(QObject *parent)
    : QObject (parent)
    , PreviewPlugin()
{

}

PDFPreviewPlugin::~PDFPreviewPlugin()
{
    if (m_pdfView)
        m_pdfView->deleteLater();
}

void PDFPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
}

bool PDFPreviewPlugin::previewItem(const ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    QFileInfo fileInfo(path);
    if (!fileInfo.isReadable())
        return false;

    if (!m_pdfView)
        m_pdfView = new PDFView(path);

    m_item = item;
    return true;
}

ItemInfo PDFPreviewPlugin::item() const
{
    return m_item;
}

QWidget *PDFPreviewPlugin::contentWidget() const
{
    return m_pdfView;
}

bool PDFPreviewPlugin::stopPreview()
{
    return true;
}

DetailInfoList PDFPreviewPlugin::getAttributeDetailInfo() const
{
    return {};
}

QWidget *PDFPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool PDFPreviewPlugin::showToolBar() const
{
    return true;
}
