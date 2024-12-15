// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreview_global.h"
#include "videopreviewinterface.h"
#include "videopreviewplugin.h"

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::video_preview;

VideoPreviewInterface::VideoPreviewInterface(QObject *parent)
  : QObject(parent)
  , PreviewPluginInterface()
{

}

PreviewPlugin *VideoPreviewInterface::create(const QString &mimetype)
{
    return new VideoPreviewPlugin();
}
