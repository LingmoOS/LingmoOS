// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreview_global.h"
#include "pdfpreviewinterface.h"
#include "pdfpreviewplugin.h"

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::pdf_preview;

PDFPreviewInterface::PDFPreviewInterface(QObject *parent)
    : QObject(parent)
    , PreviewPluginInterface()
{

}

PreviewPlugin *PDFPreviewInterface::create(const QString &mimetype)
{
    Q_UNUSED(mimetype)

    return new PDFPreviewPlugin();
}
