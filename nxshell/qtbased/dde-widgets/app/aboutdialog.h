// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"

#include <DDialog>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DTipLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

WIDGETS_FRAME_BEGIN_NAMESPACE
class InstanceAboutDialog : public DDialog {
    Q_OBJECT
public:
    explicit InstanceAboutDialog(QWidget *parent = nullptr);

    void setLogo(const QIcon &logo);
    void setVersion(const QString &version);
    void setTitle(const QString &title);
    void setDescription(const QString &description);
    void setContributor(const QString &contributor);

private:
    DLabel *m_logo = nullptr;
    DLabel *m_version = nullptr;
    DLabel *m_title = nullptr;
    DTipLabel *m_description = nullptr;
    DLabel *m_contributor = nullptr;
};

WIDGETS_FRAME_END_NAMESPACE
