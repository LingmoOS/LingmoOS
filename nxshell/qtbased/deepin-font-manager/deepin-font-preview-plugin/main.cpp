// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontpreview.h"

#ifdef DFM_BASE
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>
#else
#include "dde-file-manager/dfmfilepreviewplugin.h"
#endif

#ifdef DFM_BASE
DFMBASE_BEGIN_NAMESPACE
#else
DFM_BEGIN_NAMESPACE
#endif

#ifdef DFM_BASE
class TextPreviewPlugin : public AbstractFilePreviewPlugin
#else
class TextPreviewPlugin : public DFMFilePreviewPlugin
#endif
{
    Q_OBJECT
#ifdef DFM_BASE
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "deepin-font-preview-plugin.json")
#else
    Q_PLUGIN_METADATA(IID DFMFilePreviewFactoryInterface_iid FILE "deepin-font-preview-plugin.json")
#endif

public:
#ifdef DFM_BASE
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &) override {
        return new FontPreview();
    }
#else
    DFMFilePreview *create(const QString &key) Q_DECL_OVERRIDE {
        Q_UNUSED(key)

        return new FontPreview();
    }
#endif
};


#ifdef DFM_BASE
DFMBASE_END_NAMESPACE
#else
DFM_END_NAMESPACE
#endif

#include "main.moc"
