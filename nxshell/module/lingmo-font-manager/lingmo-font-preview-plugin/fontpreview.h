// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTPREVIEWPLUGIN_H
#define FONTPREVIEWPLUGIN_H

#include "dfontwidget.h"

#include <QObject>

#ifdef DFM_BASE
#include <dfm-base/interfaces/abstractbasepreview.h>
#else
#include "dfmfilepreview.h"
#include "durl.h"
#endif

#ifdef DFM_BASE
DFMBASE_BEGIN_NAMESPACE
#else
DFM_BEGIN_NAMESPACE
#endif

#ifdef DFM_BASE
class FontPreview : public AbstractBasePreview
#else
class FontPreview : public DFMFilePreview
#endif
{
    Q_OBJECT

public:
    explicit FontPreview(QObject *parent = nullptr);
    ~FontPreview()Q_DECL_OVERRIDE;

#ifdef DFM_BASE
    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;
#else
    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;
    DUrl fileUrl() const Q_DECL_OVERRIDE;
#endif

    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;

private:
#ifdef DFM_BASE
    QUrl m_url;
#else
    DUrl m_url;
#endif
    QString m_title;

    DFontWidget *m_previewWidget;
};

#ifdef DFM_BASE
DFMBASE_END_NAMESPACE
#else
DFM_END_NAMESPACE
#endif

#endif // FONTPREVIEWPLUGIN_H
