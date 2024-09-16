// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VMFRAME_H
#define VMFRAME_H

#include "fullscreenbackground.h"
#include "wmchooser.h"
#include "org_deepin_dde_imageblur1.h"

#include <QWidget>

using ImageBlur = org::deepin::dde::ImageBlur1;

class WMFrame : public FullscreenBackground
{
    Q_OBJECT
public:
    explicit WMFrame(QWidget *parent = 0);
    void setConfigPath(const QString &path);

private:
    WMChooser *m_wmchooser;
    ImageBlur *m_blurImageInter;
};

#endif // VMFRAME_H
