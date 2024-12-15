// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOPREVIEWINTERFACE_H
#define AUDIOPREVIEWINTERFACE_H

#include "previewplugininterface.h"

namespace GrandSearch {
namespace audio_preview {

class AudioPreviewInterface : public QObject, public PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit AudioPreviewInterface(QObject *parent = nullptr);
    virtual PreviewPlugin *create(const QString &mimetype);
};

}}

#endif // AUDIOPREVIEWINTERFACE_H
