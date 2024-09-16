// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSITEM_H
#define SETTINGSITEM_H

#include <DBackgroundGroup>
#include <DFrame>

#include <QFrame>

DWIDGET_USE_NAMESPACE

namespace def {
namespace widgets {

class SettingsItem : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool isErr READ isErr DESIGNABLE true SCRIPTABLE true)

public:
    explicit SettingsItem(QWidget *parent = nullptr);

    bool isErr() const;
    void setIsErr(const bool err = true);

    void addBackground();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    bool m_isErr;

    DTK_WIDGET_NAMESPACE::DFrame *m_bgGroup{ nullptr };
};

} // namespace widgets
} // namespace def

#endif // SETTINGSITEM_H
