// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PASSWORDLEVELWIDGET_H
#define PASSWORDLEVELWIDGET_H

#include <QWidget>

#include <dtkwidget_global.h>
#include <dwidgetstype.h>

#include <DGuiApplicationHelper>

#include "pwqualitymanager.h"

class QHBoxLayout;
DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class PasswordLevelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PasswordLevelWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    void reset();
    void setLevel(PASSWORD_LEVEL_TYPE level);

private:
    void initUI();
    void initConnections();

    QString defaultIcon();

private Q_SLOTS:
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType type);

private:
    PASSWORD_LEVEL_TYPE m_level;
    QHBoxLayout *m_layout;
    DLabel *m_tips;
    DLabel *m_lowIcon;
    DLabel *m_mediumIcon;
    DLabel *m_highIcon;
};
#endif // PASSWORDLEVELWIDGET_H
