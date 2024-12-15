// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreview_global.h"
#include "imagepreviewinterface.h"
#include "imagepreviewplugin.h"

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::image_preview;

ImagePreviewInterface::ImagePreviewInterface(QObject *parent)
    : QObject(parent)
    , PreviewPluginInterface()
{

}

PreviewPlugin *ImagePreviewInterface::create(const QString &mimetype)
{
    Q_UNUSED(mimetype)

    return new ImagePreviewPlugin();
}
