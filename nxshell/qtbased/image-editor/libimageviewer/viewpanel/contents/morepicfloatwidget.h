// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOREPICFLOATWIDGET_H
#define MOREPICFLOATWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGuiApplication>

#include <DFloatingWidget>
#include <DIconButton>
#include <DLabel>
#include <DIconButton>
#include <DWidget>
DWIDGET_USE_NAMESPACE
class MorePicFloatWidget : public DFloatingWidget
{
public:
    explicit MorePicFloatWidget(QWidget *parent = nullptr);
    ~MorePicFloatWidget();
    void initUI();

    DIconButton *getButtonUp();

    DIconButton *getButtonDown();

    void setLabelText(const QString &num);
private:
//ui
    QVBoxLayout *m_pLayout{nullptr};
    DLabel *m_labelNum{nullptr};
    DWidget *m_labelUp{nullptr};
    DWidget *m_labelDown{nullptr};
    DIconButton *m_buttonUp{nullptr};
    DIconButton *m_buttonDown{nullptr};
};

#endif // MOREPICFLOATWIDGET_H
