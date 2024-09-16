// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreview_global.h"
#include "textpreviewinterface.h"
#include "textpreviewplugin.h"

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::text_preview;

TextPreviewInterface::TextPreviewInterface(QObject *parent)
    : QObject(parent)
    , PreviewPluginInterface()
{

}

PreviewPlugin *TextPreviewInterface::create(const QString &mimetype)
{
    return new TextPreviewPlugin();
}
