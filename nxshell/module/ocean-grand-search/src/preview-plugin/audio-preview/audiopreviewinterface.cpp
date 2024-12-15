// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiopreview_global.h"
#include "audiopreviewinterface.h"
#include "audiopreviewplugin.h"

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::audio_preview;

AudioPreviewInterface::AudioPreviewInterface(QObject *parent)
    : QObject(parent)
    , PreviewPluginInterface()
{

}

PreviewPlugin *AudioPreviewInterface::create(const QString &mimetype)
{
    Q_UNUSED(mimetype)

    return new AudioPreviewPlugin();
}
